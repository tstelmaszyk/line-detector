# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Présentation

Détecteur de lignes de voie routière en C++17, basé sur OpenCV. Il
fait passer une image dans un pipeline classique de vision par ordinateur
(niveaux de gris → flou → contours Canny → masque de région d'intérêt →
transformée de Hough probabiliste) et dessine les lignes détectées sur l'image de
sortie. Il vise une configuration avec caméra Raspberry Pi, même si
`main.cpp` traite actuellement une seule image fixe lue sur le disque.

## Compilation & exécution

Build CMake hors-source (`/build` est dans le `.gitignore`) :

```sh
mkdir -p build && cd build
cmake ..
make
./line_detector        # l'exécutable s'appelle `line_detector`
```

Nécessite `OpenCV` (via `find_package`), qui doit être installé au niveau
système, sinon l'étape de configuration CMake échoue.

Il n'y a ni suite de tests, ni linter, ni CI. L'exécutable `line_detector`
est défini par `add_executable(line_detector ...)` dans `CMakeLists.txt`.

## Architecture

Le pipeline est orchestré par `DetectLines::draw_lines` (`DetectLines.cpp`),
unique point d'entrée qui enchaîne toutes les étapes dans l'ordre :

1. `grayscal` — filtre bilatéral puis conversion RGB→gris.
2. `median_blur` — réduction du bruit (note : `gaussian_blur` existe aussi mais le
   pipeline appelle actuellement `median_blur`).
3. `canny_edge_detection` — carte de contours.
4. `RegionOfInterest::apply_mask` — masque les contours hors d'un trapèze avant
   Hough, ne gardant que la zone où l'on attend les lignes de voie. **Le masque
   est appliqué sur les contours Canny, pas sur l'image finale** (cf. historique
   git — c'est un choix délibéré).
5. `hough_lines` — `HoughLinesP`, puis filtre les segments par angle
   (`compute_angle_from_two_points` > 40°) afin de ne dessiner en rouge que les
   lignes de voie quasi verticales.

Types clés et possession :

- **`VideoCaracteristics`** (`VideoCaracteristics.h`) — simple struct contenant
  `image_size`, `width_pixel`, `height_pixel`, construite à partir d'un `cv::Mat`
  de référence. C'est la source unique de la géométrie ; `DetectLines` et
  `RegionOfInterest` la reçoivent par valeur const dans leurs constructeurs. La
  créer en premier, puis la propager.
- **`DetectLines`** possède un `RegionOfInterest mask`, tous deux initialisés à
  partir de la même `VideoCaracteristics`.
- **`RegionOfInterest`** précalcule une fois le masque trapézoïdal dans son
  constructeur (`compute_trapeze_point_coordinates` + `fillPoly`) ; `apply_mask`
  applique ensuite un `bitwise_and` sur chaque image. Les sommets du trapèze
  dérivent des dimensions de l'image (ex. bord supérieur à width/2 ± width/10,
  height/2 − height/12).
- **`projectTypes.h`** — `DimensionImage` est un `typedef uint16_t`, utilisé pour
  les dimensions en pixels partout dans le code.
- **`ImageSink`** (`ImageSink.h`) — interface `bool save(name, frame)`.
  `DiskImageSink` écrit dans un dossier (`cv::imwrite`), `NullImageSink` est un
  no-op. Le résultat final et les traces de debug passent par la même interface,
  via deux instances : le résultat est toujours `DiskImageSink` ; les traces sont
  `DiskImageSink` si `LINE_DETECTOR_DEBUG` est défini, sinon `NullImageSink`. Les
  sinks sont créés dans `main` et injectés par référence dans `DetectLines` puis
  `RegionOfInterest`.

Ordre de construction dans `main.cpp` : construire `VideoCaracteristics` à partir
de l'image, puis `DetectLines`, puis appeler `draw_lines`.

## Conventions & pièges

- Le code, les commentaires et les messages de commit mélangent **français et
  anglais** ; suivre la langue du fichier environnant.
- `using namespace cv;` est utilisé dans les en-têtes — `Mat`, `Point`, etc. sont
  non qualifiés.
- Les paramètres de réglage (seuils Canny, `threshold`/`min_line_height`/
  `max_line_gap` de Hough, tailles des noyaux de flou, le filtre d'angle à 40°)
  sont codés en dur en tant que consts locales dans leurs méthodes respectives de
  `DetectLines.cpp`.
- `main.cpp` lit l'image d'entrée via un **chemin passé en argument** (`argv[1]`,
  défaut `img_piste/img2.jpg`). Le **dossier de sortie est codé en dur** dans
  `main.cpp` (`out`, source de vérité unique) ; le résultat est écrit sous
  `out/output.jpg`. L'écriture passe par un **`ImageSink`** injecté (cf.
  architecture), pas par un `imwrite` direct — pour pouvoir tourner sans écran,
  notamment en conteneur. `cv::imwrite` ne crée pas le dossier : `out/` doit
  exister (montage Docker ou `mkdir -p out`). Le code caméra (`VideoCapture`) a
  été retiré : il ne servait pas au pipeline et empêchait l'exécution hors
  Raspberry Pi.
- **Traces de debug** : exécuter avec la variable d'environnement
  `LINE_DETECTOR_DEBUG` non vide (`LINE_DETECTOR_DEBUG=1 ./line_detector`) écrit
  les étapes intermédiaires (`out/debug_00_trapeze.jpg` … `out/debug_03_canny.jpg`).
  Sans la variable, aucune trace n'est écrite (un `NullImageSink` est câblé).
  C'est un choix **runtime** : il n'y a plus d'option CMake `LINE_DETECTOR_DEBUG`.
- **Conteneurisation** : `Dockerfile` (base `debian:bookworm-slim`) installe
  OpenCV via apt et compile le projet. Construire avec
  `docker build -t line-detector .`, puis exécuter en montant un volume pour
  récupérer la sortie (cf. README/historique). `tools/make_test_image.py` génère
  une image de test `img_piste/img2.jpg` (Pillow).
- Dans `hough_lines`, la branche `else` ne fait volontairement pas
  `lines.erase(iter)` — effacer pendant l'itération provoquait un segfault (voir
  le commentaire inline).
