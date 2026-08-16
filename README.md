# line-detector

Détecteur de lignes de voie routière en C++17 (OpenCV). L'image d'entrée passe
dans un pipeline **vue de dessus (bird's eye view) + ajustement polynomial**,
capable de suivre des lignes **courbes**, et le programme produit un **signal de
pilotage** (offset latéral normalisé + rayon de courbure) destiné à maintenir un
véhicule entre les lignes. Cible visée : Raspberry Pi + caméra.

Trois sources d'entrée : une image fixe, un fichier vidéo, ou un flux caméra en
direct. Aucune fenêtre graphique n'est ouverte — le programme peut donc tourner
sans écran, notamment en conteneur.

---

## Comment ça marche

### Le problème

Une caméra embarquée voit la route **en perspective** : deux lignes de voie
parallèles y convergent vers un point de fuite, et une ligne courbe se confond
avec une ligne droite vue de biais. Mesurer directement dans cette image « où est
la voiture dans sa voie » est donc peu fiable.

L'idée du pipeline est de **supprimer la perspective avant de mesurer** :
redresser l'image en une vue de dessus, où les lignes de voie redeviennent
parallèles et où une courbe est une vraie courbe. Toutes les mesures
géométriques sont faites dans cet espace, puis le résultat est ramené sur
l'image d'origine pour l'affichage.

### Le principe, en cinq temps

```
   image caméra
        │
        │  1. isoler les marquages          →  masque binaire noir & blanc
        ▼
   masque en perspective
        │
        │  2. redresser (bird's eye view)   →  lignes parallèles et verticales
        ▼
   masque vu de dessus
        │
        │  3. suivre chaque ligne           →  nuage de pixels gauche / droite
        │  4. ajuster une courbe            →  x = a·y² + b·y + c, par côté
        │  5. en déduire la géométrie       →  offset + rayon de courbure
        ▼
   LaneModel (le signal de pilotage)
        │
        │  6. rendu (optionnel, --record)   →  overlay sur l'image d'origine
        ▼
   image annotée
```

1. **Isoler les marquages.** On ne cherche pas « des lignes », mais des pixels
   qui *ressemblent* à du marquage : blanc (forte luminance), jaune (plage de
   teinte en HSV), et bords verticaux marqués (gradient de Sobel en x). Les trois
   masques sont combinés par un OU binaire.
2. **Redresser en vue de dessus.** Un trapèze est défini dans l'image — la
   portion de route utile, plus large en bas qu'en haut — et transformé en
   rectangle par une homographie. C'est l'étape de *calibration* : bien réglée,
   deux lignes droites parallèles apparaissent verticales et parallèles dans la
   vue redressée.
3. **Suivre chaque ligne.** Un histogramme des colonnes sur la moitié basse de
   l'image donne les deux pics : les bases des lignes gauche et droite. À partir
   de là, des **fenêtres glissantes** remontent l'image de bas en haut,
   collectant les pixels allumés et se recentrant sur eux à chaque étage — ce qui
   permet de suivre une ligne qui part sur le côté.
4. **Ajuster une courbe.** Les pixels collectés de chaque côté sont ajustés par
   un polynôme du second degré `x = a·y² + b·y + c` (moindres carrés). C'est ce
   degré 2 qui permet de représenter une **courbe**, là où une transformée de
   Hough classique ne sait produire que des segments droits.
5. **En déduire le signal de pilotage.** Le milieu entre les deux polynômes,
   évalué en bas de l'image, donne le **centre de la voie** ; sa différence avec
   le centre de l'image est l'**offset latéral** (négatif = véhicule décalé à
   gauche), normalisé par la demi-largeur de voie. Les coefficients du polynôme
   donnent par ailleurs le **rayon de courbure**, `R = (1 + x'²)^1,5 / |2a|` :
   plus le terme quadratique `a` est faible, plus la voie est droite. Ces
   valeurs remplissent un `LaneModel`, émis en CSV sur `stdout` — une ligne par
   frame.

Si une seule ligne est visible (marquage effacé, ligne discontinue hors champ),
le côté manquant est **reconstruit par décalage** de la largeur de voie
attendue, et le drapeau `reconstructed` passe à `1` : le signal reste
exploitable, mais dégradé, et le module de contrôle en aval en est informé.

### Ce que le programme fait, lui

Le pipeline ci-dessus est une bibliothèque pure : elle prend une image, rend un
`LaneModel`. Autour, le programme n'est qu'un harnais — il lit les arguments,
ouvre une source (image, vidéo ou caméra), boucle frame par frame, et notifie
des *observateurs* du résultat : le log CSV, et l'écriture du fichier de sortie
si `--record` est passé. Le rendu de l'overlay n'est calculé que si un
observateur le réclame ; sans `--record`, il est purement et simplement sauté.

---

## Prérequis

OpenCV n'a pas besoin d'être installé sur la machine hôte : **tout se compile et
s'exécute dans le conteneur Docker** décrit par le `Dockerfile` (base
`debian:bookworm-slim` + `libopencv-dev`).

- [Docker](https://www.docker.com/products/docker-desktop/) installé et démarré
  (sur macOS, lancer **Docker Desktop** ; la baleine 🐳 doit être active).

### Image de test

Le dépôt ne versionne pas d'image d'entrée. Pour en générer :

```sh
python3 tools/make_test_image.py   # nécessite Pillow : pip install pillow
```

Cela crée `img_piste/img2.jpg`, `straight.jpg`, `curved.jpg`, `shifted.jpg` et
`dashed.jpg`. On peut aussi déposer sa propre photo dans `img_piste/`.

---

## Compiler et lancer

### Depuis un shell **déjà ouvert dans le conteneur**

(Dev Container VS Code, ou `docker run -it … bash` — voir plus bas.)

Se placer à la racine du projet, puis :

```sh
cmake -S . -B build-linux
cmake --build build-linux --target line_detector -j
./build-linux/line_detector --image img_piste/img2.jpg --record
```

Le résultat annoté est écrit dans `out/output.jpg`.

> **Piège — cache CMake et chemin de la source.** Le chemin du dossier source est
> figé dans `build-linux/CMakeCache.txt` (`CMAKE_HOME_DIRECTORY`). Si le projet
> a déjà été configuré depuis un autre chemin (par exemple `/app` via
> `docker run -v "$(pwd):/app"`, alors qu'un Dev Container le monte sur
> `/workspaces/line-detector`), CMake refuse de réutiliser le cache :
>
> ```
> CMake Error: The source "/workspaces/line-detector/CMakeLists.txt" does not match
> the source "/app/CMakeLists.txt" used to generate cache.
> ```
>
> Un build hors-source n'est pas relogeable : effacer le dossier de build
> (`rm -rf build-linux`) et reconfigurer, ou en utiliser un autre
> (`cmake -S . -B build-autre`).

### En une seule commande depuis l'hôte (sans entrer dans le conteneur)

Construire l'image une fois :

```sh
docker build -t line-detector .
```

Puis compiler et exécuter, en montant les sources :

```sh
docker run --rm -v "$(pwd):/app" -w /app line-detector \
  bash -c 'cmake -S /app -B /tmp/build && cmake --build /tmp/build --target line_detector -j \
           && /tmp/build/line_detector --image img_piste/img2.jpg --record'
```

Le volume `-v "$(pwd):/app"` partage le dossier du projet avec le conteneur :
modifier le code ou déposer une nouvelle image dans `img_piste/` ne demande
aucun rebuild de l'image Docker, et `out/` apparaît directement sur l'hôte.

### Boucle de développement interactive

```sh
docker run --rm -it -v "$(pwd):/app" -w /app line-detector bash
# puis, À L'INTÉRIEUR du conteneur :
cmake -S . -B build-linux && cmake --build build-linux --target line_detector -j
./build-linux/line_detector --image img_piste/img2.jpg --record
```

### Avec VS Code (Dev Containers)

1. Installer l'extension **Dev Containers** (`ms-vscode-remote.remote-containers`).
2. `Cmd/Ctrl + Shift + P` → **« Dev Containers: Reopen in Container »**.
3. Ouvrir un terminal intégré et utiliser les commandes de la section
   « shell déjà ouvert dans le conteneur » ci-dessus.

Le dossier de build est `build-linux/` (réglé dans
`.devcontainer/devcontainer.json`) pour ne pas mélanger le cache CMake Linux
avec celui d'un éventuel build local macOS.

### Sans conteneur

Sur une machine où OpenCV est installé au niveau système, le build générique
fonctionne aussi :

```sh
mkdir build && cd build && cmake .. && make
```

---

## Utilisation

```
line_detector [--image <chemin> | --video <chemin> | --camera [index]] [--record]
```

Les trois modes sont **mutuellement exclusifs** :

| Mode | Effet | Sortie avec `--record` |
|---|---|---|
| `--image <chemin>` | traite une image fixe (une frame) | `out/output.jpg` |
| `--video <chemin>` | traite un fichier vidéo | `out/output.avi` |
| `--camera [index]` | traite un flux caméra en direct (index par défaut : `0`) | `out/output.avi` |

**`--record` gouverne l'écriture du résultat**, uniformément pour les trois
modes — l'image fixe est un cas dégénéré du flux, pas une exception. Sans
`--record`, le programme n'écrit **aucun fichier** et n'exécute **aucun rendu** :
aucun observateur ne réclamant l'image annotée, l'étape d'overlay est purement et
simplement sautée (la colonne `render_ms` reste à `0`). C'est le mode à utiliser
pour consommer le signal de pilotage au coût minimal.

`Ctrl-C` arrête proprement la boucle et referme le fichier vidéo ; un second
`Ctrl-C` termine toujours le processus, même si la lecture caméra est bloquée.

### Sorties : `stdout` et `stderr` sont séparés

**`stdout`** ne porte **que** une ligne d'en-tête puis une ligne CSV par frame
(séparateur `;`, format `std::fixed`, jamais de notation scientifique) :

```
frame_index;lane_detected;normalized_offset;lateral_offset_px;curvature_radius_px;reconstructed;compute_ms;render_ms
```

**`stderr`** porte tout ce qui s'adresse à un humain : erreurs, résumé final
(frames / détections / reconstructions / ms par frame / FPS) et, si `--record`,
le chemin du fichier écrit.

Rediriger `stdout` seul suffit donc à consommer le signal de pilotage sans
parser de texte :

```sh
./build-linux/line_detector --camera 0 > pilotage.csv
```

Le signe de l'offset : **négatif = véhicule décalé à gauche**. La colonne
`reconstructed` vaut `1` quand une seule ligne était visible et que l'autre a été
reconstruite par décalage — le signal est alors dégradé.

### Dossier de sortie

Par défaut `out/`, surchargeable par la variable d'environnement
`LINE_DETECTOR_OUT`. Le programme crée le dossier lui-même si nécessaire (et
uniquement s'il a quelque chose à y écrire).

### Cadence de la vidéo de sortie

La vidéo annotée est toujours écrite à une cadence **fixe de 30 fps**, quelle que
soit la cadence réelle de la source. Une vidéo issue d'une source plus lente ou
plus rapide paraîtra donc accélérée ou ralentie à la relecture — ce n'est pas un
bug.

---

## Traces de debug

Exécuter avec la variable d'environnement `LINE_DETECTOR_DEBUG` non vide écrit
les étapes intermédiaires dans le dossier de sortie :

```sh
LINE_DETECTOR_DEBUG=1 ./build-linux/line_detector --image img_piste/img2.jpg --record
```

| Fichier | Étape | Condition |
|---|---|---|
| `out/debug_01_mask.jpg` | masque des marquages | `LINE_DETECTOR_DEBUG` |
| `out/debug_02_bev.jpg` | vue de dessus | `LINE_DETECTOR_DEBUG` |
| `out/debug_03_windows.jpg` | fenêtres glissantes | `LINE_DETECTOR_DEBUG` |
| `out/debug_04_fit.jpg` | polynômes ajustés | `LINE_DETECTOR_DEBUG` |
| `out/debug_05_overlay.jpg` | overlay final | `LINE_DETECTOR_DEBUG` **et** `--record` |

`debug_05_overlay.jpg` vient de l'étape de rendu, qui n'est exécutée que si un
observateur réclame l'image annotée — d'où la condition supplémentaire.

En mode flux, les noms de fichiers sont fixes : chaque frame écrase la
précédente, la dernière gagne. C'est voulu — ces traces servent à caler la
calibration, pas à archiver une séquence.

**Calibration de la vue de dessus** : régler les ratios `src_top_width_ratio` /
`src_top_y_ratio` / `bev_margin_ratio` de `LaneConfig.h` en inspectant
`out/debug_02_bev.jpg`, jusqu'à ce que des lignes droites parallèles y
apparaissent verticales et parallèles. Non bloquant : les valeurs par défaut
fonctionnent, seule la précision géométrique est dégradée.

---

## Tests

Suite de tests unitaires **doctest** (header vendored dans `tests/doctest.h`),
cible CMake séparée `line_detector_tests` :

```sh
cmake -S . -B build-linux
cmake --build build-linux --target line_detector_tests -j
./build-linux/line_detector_tests
```

Ou depuis l'hôte, en une commande :

```sh
docker run --rm -v "$(pwd):/app" -w /app line-detector \
  bash -c 'cmake -S /app -B /tmp/build && cmake --build /tmp/build --target line_detector_tests -j \
           && /tmp/build/line_detector_tests'
```

Chaque composant du pipeline et de la couche application a ses tests, plus trois
tests bout-en-bout : deux sur des images synthétiques (dont une courbe) et un sur
une séquence vidéo où la voie dérive progressivement, qui vérifie que
`normalized_offset` évolue de façon monotone. Pas de linter ni de CI.

---

## Architecture

Les étapes décrites dans « Comment ça marche », côté code. `DetectLines`
orchestre le tout et expose deux points d'entrée : **`compute()`** (étapes 1 à 5,
renvoie le `LaneModel` sans rien dessiner) et **`render()`** (étape 6, dessine
l'overlay à partir d'un `LaneModel` déjà calculé).

1. **`LaneMask`** — masque binaire des marquages : blanc (seuil de luminance),
   jaune (`inRange` en HSV), bords (Sobel x), combinés par OU binaire.
2. **`PerspectiveView::toBev`** — warp perspective vers la vue de dessus.
3. **`SlidingWindowSearch`** — histogramme des colonnes pour trouver les deux
   bases, puis fenêtres glissantes de bas en haut collectant les pixels.
4. **`LanePolynomial::fit`** — ajuste `x = a·y² + b·y + c` par moindres carrés.
5. **`LaneGeometry::compute`** — centre de voie, offset latéral, offset
   normalisé, rayon de courbure ; reconstruit un côté manquant par décalage.
6. **`LaneOverlay::render`** — polygone de voie en vue de dessus, ramené en
   perspective image, fusionné sur l'image d'origine, plus un HUD.

### Couche application

Deux cibles CMake matérialisent la frontière :

- **`line_detector_lib`** — le pipeline ci-dessus. Ne lit pas `argv`, n'écrit pas
  sur `stdout`, ne possède aucune boucle de capture.
- **`line_detector_app`** — le harnais : analyse des arguments (`CliOptions`),
  sources de frames (`FrameSource`), observateurs (`FrameObserver`) et boucle
  (`PipelineRunner`). C'est la cible que l'exécutable **et** les tests linkent,
  pour garantir qu'ils exercent exactement le même code.

---

## Structure du projet

| Fichier / dossier | Rôle |
|---|---|
| `main.cpp` | assemblage : arguments, source, détecteur, observateurs, boucle |
| `DetectLines.*` | orchestration du pipeline (`compute` / `render`) |
| `LaneMask.*`, `PerspectiveView.*`, `SlidingWindowSearch.*` | étapes 1 à 3 |
| `LanePolynomial.*`, `LaneGeometry.*`, `LaneOverlay.*` | étapes 4 à 6 |
| `LaneConfig.h` | **tous** les réglages (seuils, calibration, fenêtres) |
| `LaneModel.h` | le signal de pilotage produit |
| `CliOptions.*`, `FrameSource.h`, `FrameObserver.h`, `PipelineRunner.*` | couche application |
| `ImageSink.h`, `DiskImageSink.*`, `NullImageSink.h` | écriture des images (résultat et debug) |
| `tests/` | suite doctest (`line_detector_tests`) |
| `CMakeLists.txt` | cibles `line_detector_lib`, `line_detector_app`, exécutable, tests |
| `Dockerfile`, `.dockerignore`, `.devcontainer/` | environnement de build reproductible |
| `tools/make_test_image.py` | génère les images de test dans `img_piste/` |

---

## Suite prévue

`LaneTracker` (lissage temporel / Kalman), recherche autour du fit précédent,
correction de distorsion caméra, passage à une sortie métrique.
