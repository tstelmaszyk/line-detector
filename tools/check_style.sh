#!/usr/bin/env bash
# Vérifie les règles de formatage objectives de coding_rules.md sur le code
# source métier et les tests : pas de tabulation, pas d'espace en fin de ligne,
# pas de CRLF, dernière ligne vide. À lancer dans le conteneur (GNU grep/bash).
set -euo pipefail

# Racine du projet : variable d'environnement ou répertoire parent du script.
project_root="${PROJECT_ROOT:-$( cd "$( dirname "$0" )/.." && pwd )}"

status=0

# .h/.cpp à la racine + tests test_*.cpp (le header vendored tests/doctest.h est exclu).
mapfile -t files < <( find "$project_root" -maxdepth 1 -type f \( -name '*.h' -o -name '*.cpp' \) | sort )
mapfile -t -O "${#files[@]}" files < <( find "$project_root/tests" -maxdepth 1 -type f -name 'test_*.cpp' | sort )

for file in "${files[@]}"; do
  if grep -nP '\t' "$file" >/dev/null; then
    echo "TABULATION: $file"
    status=1
  fi

  if grep -nP ' +$' "$file" >/dev/null; then
    echo "ESPACE FIN DE LIGNE: $file"
    status=1
  fi

  if grep -nP '\r$' "$file" >/dev/null; then
    echo "CRLF: $file"
    status=1
  fi

  if [ -n "$( tail -c 1 "$file" )" ]; then
    echo "PAS DE LIGNE VIDE FINALE: $file"
    status=1
  fi
done

if [ "$status" -eq 0 ]; then
  echo "Style OK"
fi

exit "$status"
