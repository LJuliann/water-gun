<div align="center">

# 💧 Water Gun

Projet de pistolet à eau sur **Raspberry Pi Pico 2 · RP2350**.

![C](https://img.shields.io/badge/langage-C-00599C?style=flat-square)
![Pico 2](https://img.shields.io/badge/Raspberry_Pi-Pico_2-A22846?style=flat-square)
![Statut](https://img.shields.io/badge/statut-en_développement-38BDF8?style=flat-square)

</div>

## Le projet

Base du firmware en C, construite avec CMake et le Pico SDK. Pour l’instant, le programme initialise les entrées/sorties standard et reste dans une boucle d’attente ; le contrôle matériel reste à implémenter.

## Compiler

Prérequis : **CMake ≥ 3.21**, une chaîne de compilation **Arm embarquée** et le **Pico SDK** avec ses sous-modules initialisés.

```sh
export PICO_SDK_PATH=/chemin/vers/pico-sdk
cmake -S . -B build -DPICO_BOARD=pico2
cmake --build build
```

Pour flasher la carte, maintenir **BOOTSEL** pendant le branchement USB, puis copier `build/water_gun.uf2` sur le volume de la Pico 2.

## Fichiers

- [`water_gun.c`](water_gun.c) — point d’entrée du firmware.
- [`CMakeLists.txt`](CMakeLists.txt) — configuration de compilation.
- [`pico_sdk_import.cmake`](pico_sdk_import.cmake) — import du Pico SDK.
