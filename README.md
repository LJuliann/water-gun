<div align="center">

# Water Gun

Un projet de pistolet à eau piloté par Raspberry Pi Pico 2.

**C** &nbsp; / &nbsp; **RP2350** &nbsp; / &nbsp; **Pico SDK**

</div>

---

En développement : la base du firmware est en place ; le contrôle matériel reste à implémenter.

## Compiler

Prérequis : CMake ≥ 3.21, une chaîne de compilation Arm embarquée et le Pico SDK avec ses sous-modules initialisés.

Sur CachyOS / Arch Linux :

```sh
sudo pacman -S --needed cmake ninja arm-none-eabi-gcc arm-none-eabi-newlib base-devel python libusb
git clone --branch 2.3.1 --depth 1 --recurse-submodules --shallow-submodules https://github.com/raspberrypi/pico-sdk.git .tools/pico-sdk
```

Le SDK dans `.tools/pico-sdk` est détecté automatiquement et ignoré par Git.
Pour compiler depuis le terminal ou le preset CMake de VS Code :

```sh
cmake --preset pico2
cmake --build --preset pico2
```

La première compilation peut télécharger et compiler `picotool` ; une connexion Internet est nécessaire.
Le firmware est généré dans `build/water_gun.uf2`, au chemin attendu par Wokwi.

Pour utiliser un SDK installé ailleurs :

```sh
export PICO_SDK_PATH=/chemin/vers/pico-sdk
cmake -S . -B build -DPICO_BOARD=pico2
cmake --build build
```

## Installer

Maintenir **BOOTSEL** pendant le branchement USB de la Pico 2, puis copier `build/water_gun.uf2` sur le volume qui apparaît.
