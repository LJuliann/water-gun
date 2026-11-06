<div align="center">

# Water Gun

Un projet de pistolet à eau piloté par Raspberry Pi Pico 2.

**C** &nbsp; / &nbsp; **RP2350** &nbsp; / &nbsp; **Pico SDK**

</div>

---

En développement : la base du firmware est en place ; le contrôle matériel reste à implémenter.

## Compiler

Prérequis : CMake ≥ 3.21, une chaîne de compilation Arm embarquée et le Pico SDK avec ses sous-modules initialisés.

```sh
export PICO_SDK_PATH=/chemin/vers/pico-sdk
cmake -S . -B build -DPICO_BOARD=pico2
cmake --build build
```

## Installer

Maintenir **BOOTSEL** pendant le branchement USB de la Pico 2, puis copier `build/water_gun.uf2` sur le volume qui apparaît.
