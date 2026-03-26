---
lang: fr
---

# Synthétiseur depuis Température et Humidité

Projet de synthétiseur modulant le son basé sur les températures et humidités
mesurées par deux capteurs, sur RTOS MBED-CE.

| Must                                                               |
| ------------------------------------------------------------------ |
| Captage de deux températures distinctes                            |
| Définition de la température ambiante (état de repos) au démarrage |
| Génération audio à partir des températures                         |

| Should                                                                   |
| ------------------------------------------------------------------------ |
| Captage de deux humidités                                                |
| Prise en compte des humidités dans la synthèse audio                     |
| Assignation aléatoire des inputs aux paramètres de synthèse au démarrage |

| Could                                                                      |
| -------------------------------------------------------------------------- |
| Produire une vingtaine de prototypes, donner concert au palais des congrès |

| Won’t                                      |
| ------------------------------------------ |
| Permettre de jouer un morceau précis       |
| Permettre de jouer deux fois la même chose |
| Fonctionner de manière prédictible         |

## Matériel

| Fonction                       | Composant                                  |
| ------------------------------ | ------------------------------------------ |
| Captage température & humidité | [Grove temperature & humidity sensor v1.0] |
| Captage température & humidité | [Adafruit BME280 I2C or SPI]               |
| Amplification & émission son   | [Adafruit PAM8302 Amplifier]               |
| Traitements numériques         | [Kit MBED NXP LPC1768]                     |

## Outillage

| Fonction                     | Outil                    |
| ---------------------------- | ------------------------ |
| Compilation C++              | [GCC ARM]                |
| Système de build             | [CMake] + [Ninja]        |
| Dépendances et environnement | [Nix]                    |
| Versionnage et collaboration | [Git] hébergé sur GitHub |
| Tests automatisés            | [doctest]                |
| Couverture de code           | [GCov]                   |
| Assistance langage C++       | [clangd] (LSP)           |
| Documentation depuis le code | [Doxygen]                |
| Formatage du C++             | [clang-format]           |
| Contrôle qualité C++         | [clang-tidy]             |
| Débogage C++                 | [GDB]                    |
| Édition du code              | [VS Code], [Helix]…      |

## Compilation & Exécution

Cet environnement utilise [Nix] pour télécharger les (bonnes versions des)
dépendances, configurer l’environnement, et permettre in-fine d’effectuer des
compilations (possiblement croisées) reproductibles. L’environnement [Nix] est
défini dans [`flake.nix`](./flake.nix) et s’active avec la commande
`nix flake develop` (`nix` doit être installé) ou plus simplement avec [direnv]
via `direnv allow` (qui doit aussi être installé séparément) seulement la
première fois (ensuite automatique).

Pour compiler le projet, utiliser [CMake] via `cmake --build build` directement
depuis un environnement [Nix] activé.

> Pour accélérer les opérations impliquant `cmake`, indiquer le nombre `N` de
> threads correspondant au nombre de cœurs de processeur avec `-jN`, par exemple
> `cmake --build build -j4` pour quatre cœurs.

[Adafruit PAM8302 Amplifier]: https://www.adafruit.com/product/2130
[Adafruit BME280 I2C or SPI]: https://www.adafruit.com/product/2652
[cmake]: https://cmake.org
[clang]: https://clang.llvm.org
[clangd]: https://clangd.llvm.org
[clang-format]: https://clangd.llvm.org
[clang-tidy]: https://clangd.llvm.org
[C++23]: https://en.wikipedia.org/wiki/C%2B%2B23
[direnv]: https://direnv.net
[doctest]: https://github.com/doctest/doctest
[doxygen]: https://www.doxygen.nl
[gcc]: https://gcc.gnu.org
[gcc arm]: https://gcc.gnu.org
[gcov]: https://gcc.gnu.org/onlinedocs/gcc/Gcov.html
[gdb]: https://www.gnu.org/software/gdb
[git]: https://git-scm.com
[Grove temperature & humidity sensor v1.0]: https://wiki.seeedstudio.com/Grove-TemptureAndHumidity_Sensor-High-Accuracy_AndMini-v1.0
[helix]: https://helix-editor.com
[Kit MBED NXP LPC1768]: https://os.mbed.com/platforms/mbed-LPC1768
[lcov]: https://github.com/linux-test-project/lcov
[lldb]: https://lldb.llvm.org
[llvm-cov]: https://llvm.org/docs/CommandGuide/llvm-cov.html
[nix]: https://nixos.org
[ninja]: https://ninja-build.org
[polytech]: https://polytech.univ-tours.fr
[polytech tours]: https://polytech.univ-tours.fr
[socat]: http://www.dest-unreach.org/socat
[tio]: https://github.com/tio/tio
[vscode]: https://code.visualstudio.com
[vs code]: https://code.visualstudio.com
