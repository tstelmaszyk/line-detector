# line-detector

Détecteur de lignes de voie routière en C++17 (OpenCV + libcamera). L'image
d'entrée passe dans un pipeline de vision par ordinateur (gris → flou → contours
Canny → masque de région d'intérêt → transformée de Hough) et les lignes
détectées sont dessinées sur l'image de sortie.

`main.cpp` lit une image sur disque et **écrit le résultat dans un fichier**
(`imwrite`) — pas de fenêtre graphique — afin de pouvoir tourner sans écran,
notamment en conteneur.

```
./test <image_entree> <image_sortie>
# défauts : img_piste/img2.jpg  et  output.jpg
```

---

## Pourquoi un conteneur ?

Le projet dépend de **libcamera**, une bibliothèque Linux (Raspberry Pi) qui ne
s'installe pas sur macOS/Windows. Docker fournit un mini-Linux jetable où OpenCV
et libcamera s'installent proprement, ce qui permet de compiler et d'exécuter le
projet depuis n'importe quelle machine.

**Principe directeur :** on *fige* dans l'image ce qui change rarement (compilateur,
OpenCV, libcamera) et on *monte en volume* ce qui change souvent (images de test,
et — pendant le développement — le code source). Un *volume* est un dossier
partagé entre la machine hôte et le conteneur.

### Prérequis
- [Docker](https://www.docker.com/products/docker-desktop/) installé et démarré
  (sur macOS, lancer **Docker Desktop** ; la baleine 🐳 doit être active).

### Image de test
Le dépôt ne versionne pas d'image d'entrée. Pour en générer une (route avec deux
lignes de voie) :

```sh
python3 tools/make_test_image.py   # nécessite Pillow : pip install pillow
```

Cela crée `img_piste/img2.jpg`. Tu peux aussi déposer ta propre photo dans
`img_piste/`.

---

## Méthode 1 — Avec VS Code (Dev Containers) — *recommandé*

Développer **à l'intérieur** du conteneur : éditeur, terminal, compilation et
débogueur tournent dans l'environnement Linux. Aucune commande `docker` à taper,
et aucun rebuild de l'image quand on modifie le code.

1. Installer l'extension **Dev Containers** (`ms-vscode-remote.remote-containers`).
2. S'assurer que Docker Desktop tourne.
3. `Cmd/Ctrl + Shift + P` → **« Dev Containers: Reopen in Container »**.
   - La première fois, VS Code construit l'image depuis le `Dockerfile`
     (quelques minutes) ; ensuite c'est quasi instantané grâce au cache.
4. Une fois dans le conteneur (badge « Dev Container » en bas à gauche), ouvrir
   un terminal intégré et compiler/lancer :

```sh
cmake -S . -B build-linux && cmake --build build-linux
./build-linux/test img_piste/img2.jpg output.jpg
```

`output.jpg` apparaît dans l'explorateur VS Code.

**Boucle de développement :** éditer le code → `cmake --build build-linux` →
relancer. Les fichiers sont partagés avec la machine hôte (volume), donc les
modifications sont immédiates — pas de rebuild d'image.

> La compilation utilise `build-linux/` (et non `build/`) pour ne pas mélanger le
> cache CMake de Linux avec celui d'un éventuel build local macOS. Ce réglage est
> dans `.devcontainer/devcontainer.json`.

---

## Méthode 2 — Sans VS Code (Docker en ligne de commande)

### A. Construire l'image (une fois, ou après modif du code)

```sh
docker build -t line-detector .
```

`-t line-detector` nomme l'image ; le `.` indique où trouver le `Dockerfile`.
Grâce au cache de couches, modifier le code ne réinstalle pas OpenCV/libcamera :
seules les étapes de copie et de compilation sont rejouées.

### B. Exécuter et récupérer le résultat (via un volume)

```sh
mkdir -p out
docker run --rm \
  -v "$(pwd)/img_piste:/app/img_piste" \
  -v "$(pwd)/out:/app/out" \
  line-detector ./build/test img_piste/img2.jpg out/output.jpg
```

- `--rm` : supprime le conteneur à la fin.
- `-v "<hôte>:<conteneur>"` : monte un dossier de l'hôte dans le conteneur.
  - `img_piste/` est monté → tester une **nouvelle image** ne demande aucun
    rebuild : on la dépose dans `img_piste/` et on change l'argument.
  - `out/` est monté → l'image de sortie écrite dans le conteneur apparaît sur
    l'hôte, dans `out/output.jpg`.

Ouvrir ensuite `out/output.jpg`.

### C. (Optionnel) Boucle de développement sans VS Code

Monter le code source et compiler dans le conteneur, sans rebuild d'image :

```sh
docker run --rm -it -v "$(pwd):/app" line-detector bash
# puis, À L'INTÉRIEUR du conteneur :
cmake -S . -B build-linux && cmake --build build-linux
./build-linux/test img_piste/img2.jpg output.jpg
```

(`-it` ouvre un terminal interactif. On compile dans `build-linux/` pour ne pas
entrer en conflit avec un build local macOS.)

---

## Structure du projet

| Fichier / dossier | Rôle |
|---|---|
| `main.cpp` | point d'entrée : lit l'image, lance la détection, écrit la sortie |
| `DetectLines.*` | pipeline de détection (gris, flou, Canny, Hough) |
| `RegionOfInterest.*` | masque trapézoïdal appliqué aux contours |
| `VideoCaracteristics.h` | géométrie de l'image (dimensions) |
| `CMakeLists.txt` | configuration de build (OpenCV + libcamera) |
| `Dockerfile` / `.dockerignore` | recette du conteneur |
| `.devcontainer/` | config VS Code Dev Containers |
| `tools/make_test_image.py` | génère une image de test |
