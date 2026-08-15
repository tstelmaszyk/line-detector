# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Présentation

Détecteur de lignes de voie routière en C++17, basé sur OpenCV. Il fait passer
une image dans un pipeline **vue de dessus (bird's eye view) + ajustement
polynomial** capable de suivre des lignes **courbes**, et produit un **signal de
pilotage** (offset latéral normalisé + rayon de courbure) destiné à maintenir un
véhicule entre les lignes. Il vise une configuration avec caméra Raspberry Pi,
même si `main.cpp` traite actuellement une seule image fixe lue sur le disque
(l'architecture est prête pour un flux vidéo — cf. roadmap dans
`docs/superpowers/specs/`).

## Compilation & exécution

Build CMake hors-source. **OpenCV n'est pas installé sur l'hôte de dev (macOS)** :
la compilation se fait dans le conteneur Docker (image `line-detector`, base
`debian:bookworm-slim` + `libopencv-dev`). L'image se construit avec
`docker build -t line-detector .`.

Compiler l'exécutable et lancer sur une image, en montant la source :

```sh
docker run --rm -v "$(pwd):/app" -w /app line-detector \
  bash -c 'cmake -S /app -B /tmp/build && cmake --build /tmp/build --target line_detector -j \
           && mkdir -p /app/out && cd /app && /tmp/build/line_detector --image img_piste/img2.jpg'
```

Trois modes, mutuellement exclusifs : `--image <chemin>` (défaut : `img_piste/img2.jpg`),
`--video <chemin>` (fichier vidéo) et `--camera [index]` (défaut `0`). Le mode image
écrit `out/output.jpg` ; les modes flux écrivent `out/output.avi`. Dans tous les cas,
une ligne CSV par frame est imprimée sur la sortie standard
(`frame_index;lane_detected;normalized_offset;lateral_offset_px;curvature_radius_px;reconstructed;elapsed_ms`),
suivie d'un résumé (frames, détections, ms/frame, FPS). `Ctrl-C` arrête proprement
la boucle et ferme le fichier vidéo.

Sur une machine où OpenCV est installé au niveau système, le build générique
fonctionne aussi (`mkdir build && cd build && cmake .. && make`) ; l'exécutable
s'appelle `line_detector` (`add_executable(line_detector ...)` dans
`CMakeLists.txt`).

## Tests

Il existe une suite de tests unitaires **doctest** (header unique vendored dans
`tests/doctest.h`). Cible CMake séparée `line_detector_tests` ; les fichiers
`tests/test_*.cpp` sont ramassés par `file(GLOB ...)` (un nouveau test est pris en
compte car le build reconfigure via `cmake -S -B`). Lancer :

```sh
docker run --rm -v "$(pwd):/app" -w /app line-detector \
  bash -c 'cmake -S /app -B /tmp/build && cmake --build /tmp/build --target line_detector_tests -j \
           && /tmp/build/line_detector_tests'
```

Chaque composant du pipeline a ses tests ; deux tests bout-en-bout
(`tests/test_integration.cpp`) exercent tout `DetectLines` sur des images
synthétiques (dont une courbe qui vérifie que le fit n'est pas aplati). Pas de
linter ni de CI.

## Architecture

Le pipeline est orchestré par `DetectLines::draw_lines` (`DetectLines.cpp`),
unique point d'entrée. Il **renvoie un `LaneModel`** (le signal de pilotage) et
dessine le résultat dans l'image de sortie. Étapes, dans l'ordre :

1. **`LaneMask`** (`LaneMask.cpp`) — image binaire des marquages depuis le BGR :
   blanc (seuil de luminance sur `COLOR_BGR2GRAY`), jaune (`inRange` en HSV),
   bords (Sobel x), combinés par OU binaire. Remplace l'ancien trio
   grayscale/flou/Canny. **Utilise `COLOR_BGR2GRAY`** (`imread` charge en BGR).
2. **`PerspectiveView::toBev`** (`PerspectiveView.cpp`) — warp perspective vers une
   vue de dessus via `getPerspectiveTransform`. Le quadrilatère source (trapèze
   dérivé de `VideoCaracteristics` + `LaneConfig`) hérite de la géométrie de
   l'ancien `RegionOfInterest` ; `warpBack` fait la transformation inverse.
3. **`SlidingWindowSearch::search`** (`SlidingWindowSearch.cpp`) — histogramme des
   colonnes (moitié basse) pour trouver les deux bases, puis fenêtres glissantes
   de bas en haut collectant les pixels gauche/droite (`LanePixels`). Une fenêtre
   sans assez de pixels ne se recentre pas.
4. **`LanePolynomial::fit`** (`LanePolynomial.cpp`) — ajuste `x = a·y² + b·y + c`
   par moindres carrés (`cv::solve`, Vandermonde) pour chaque côté.
5. **`LaneGeometry::compute`** (`LaneGeometry.cpp`) — remplit le `LaneModel` :
   centre de voie, offset latéral, offset normalisé, rayon de courbure. Reconstruit
   un côté manquant par décalage (cf. plus bas).
6. **`LaneOverlay::render`** (`LaneOverlay.cpp`) — remplit le polygone de voie en
   BEV, le ramène en perspective image (`warpBack`), le fusionne sur l'image
   d'origine et ajoute un HUD (offset + courbure).

Types clés et possession :

- **`VideoCaracteristics`** (`VideoCaracteristics.h`) — struct contenant
  `image_size`, `width_pixel`, `height_pixel`, construite depuis un `cv::Mat` de
  référence. Source unique de la géométrie ; propagée par valeur const à chaque
  composant. La créer en premier.
- **`LaneConfig`** (`LaneConfig.h`) — struct centralisant **tous** les réglages
  (seuils blanc/jaune/Sobel, ratios de calibration BEV, fenêtres glissantes,
  `defaultLaneWidthPx`). Construite dans `main`, injectée par valeur const. C'est
  le point de réglage unique (notamment la calibration perspective).
- **`LaneModel`** (`LaneModel.h`) — résultat : deux `LanePolynomial` (gauche/droite,
  en pixels BEV), `laneDetected`, le signal (`lateralOffsetPx`,
  `normalizedOffset`, `curvatureRadiusPx`) et `reconstructed`. **Signe de
  l'offset : négatif = véhicule décalé à gauche.**
- **`LanePolynomial`** (`LanePolynomial.h`) — polynôme `x = a·y² + b·y + c` avec un
  drapeau `valid` et une fabrique statique `fit(points, minPoints)`.
- **`DetectLines`** possède `LaneMask mask`, `PerspectiveView perspective`,
  `SlidingWindowSearch search`, `LaneOverlay overlay` — tous initialisés depuis la
  même `VideoCaracteristics`/`LaneConfig`. **`overlay` est déclaré après
  `perspective`** car il détient une `const PerspectiveView&` (l'ordre de
  déclaration = ordre de construction).
- **`projectTypes.h`** — `DimensionImage` est un `typedef uint16_t`, utilisé pour
  les dimensions en pixels.
- **`ImageSink`** (`ImageSink.h`) — interface `bool save(name, frame)`.
  `DiskImageSink` écrit dans un dossier (`cv::imwrite`), `NullImageSink` est un
  no-op. Résultat final et traces de debug passent par la même interface : le
  résultat est toujours `DiskImageSink` ; les traces sont `DiskImageSink` si
  `LINE_DETECTOR_DEBUG` est défini, sinon `NullImageSink`. Les sinks sont créés
  dans `main` et injectés par référence dans `DetectLines` puis les composants.

Ordre de construction dans `main.cpp` : `VideoCaracteristics` depuis l'image,
puis `LaneConfig`, puis `DetectLines`, puis `draw_lines` (dont on lit le
`LaneModel` renvoyé).

## Design by contract

Distinction stricte, appliquée dans tout le pipeline :

- **Violation d'invariant de code** (ne devrait jamais arriver si le code est
  correct : `Mat` vide, mauvais nombre de canaux, taille ≠ `VideoCaracteristics`,
  homographie dégénérée, offset non fini) → **`SMART_ASSERT`** (`SmartAssert.h`),
  macro **toujours active** (non supprimée par `NDEBUG`, contrairement à `assert`).
- **Aléa de la route** (voie absente, trop peu de pixels, largeur non plausible)
  → **drapeau** (`LanePolynomial::valid`, `LaneModel::laneDetected`), jamais un
  assert. Un côté manquant est reconstruit par décalage (`LaneModel::reconstructed`
  passe à `true` — signal dégradé, à signaler au module de contrôle).

## Conventions & pièges

- Le code, les commentaires et les messages de commit mélangent **français et
  anglais** ; suivre la langue du fichier environnant.
- Le **nouveau code qualifie explicitement `cv::`**. Certains en-têtes hérités
  contiennent encore `using namespace cv;`.
- Les réglages ne sont plus des consts éparpillées : ils vivent dans **`LaneConfig`**.
- `main.cpp` n'est plus qu'un assemblage : il analyse les arguments (`parse_arguments`),
  construit la `FrameSource`, lit la **première frame** (c'est elle qui définit
  `VideoCaracteristics`, jamais les métadonnées de `cv::VideoCapture`), construit le
  détecteur et les observateurs, puis délègue la boucle à `PipelineRunner`. La
  bibliothèque `line_detector_lib` ne lit pas `argv`, n'écrit pas sur `stdout` et ne
  possède aucune boucle : `cv::VideoCapture` vit côté application.
- **Traces de debug en mode flux** : les noms de fichiers sont fixes, donc chaque frame
  écrase la précédente — la dernière frame gagne. Voulu : les traces servent à caler la
  calibration BEV, pas à archiver la séquence.
- **Calibration BEV** : les 4 points source dérivent des ratios
  `srcTopWidthRatio` / `srcTopYRatio` / `bevMarginRatio` de `LaneConfig`. La régler
  en inspectant `out/debug_02_bev.jpg` jusqu'à ce que des lignes droites
  parallèles apparaissent verticales et parallèles. **Non bloquant** : les valeurs
  par défaut fonctionnent (précision géométrique dégradée seulement).
- **Traces de debug** : exécuter avec `LINE_DETECTOR_DEBUG` non vide
  (`LINE_DETECTOR_DEBUG=1 ./line_detector`) écrit les étapes intermédiaires :
  `out/debug_01_mask.jpg`, `debug_02_bev.jpg`, `debug_03_windows.jpg`,
  `debug_04_fit.jpg`, `debug_05_overlay.jpg`. Sans la variable, aucune trace
  (`NullImageSink`). Choix **runtime** : pas d'option CMake.
- **Conteneurisation** : `Dockerfile` (base `debian:bookworm-slim`) installe
  OpenCV via apt et compile le projet. `tools/make_test_image.py` (Pillow) génère
  des images de test dans `img_piste/` : `img2.jpg`, `straight.jpg`, `curved.jpg`,
  `shifted.jpg`, `dashed.jpg`.
- **Code retiré** : l'ancien pipeline Canny/Hough (`grayscal`, `median_blur`,
  `canny_edge_detection`, `hough_lines`, `compute_angle_from_two_points`) et la
  classe `RegionOfInterest` (son trapèze est absorbé par `PerspectiveView`). Le
  code caméra (`VideoCapture`) avait déjà été retiré.

## Suite prévue

Voir `docs/superpowers/specs/2026-07-10-roadmap-video-lissage-temporel.md` : mode
vidéo, `LaneTracker` (lissage temporel / Kalman), recherche autour du fit
précédent, correction de distorsion caméra, passage métrique.
