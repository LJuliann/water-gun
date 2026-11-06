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

Après un changement d'installation du SDK (par exemple via l'extension Pico),
recréer le cache pour éviter de mélanger les chemins de deux SDK :

```sh
cmake --fresh --preset pico2
cmake --build --preset pico2
```

L'option `--fresh` nécessite CMake ≥ 3.24.

Pour utiliser un SDK installé ailleurs :

```sh
export PICO_SDK_PATH=/chemin/vers/pico-sdk
cmake -S . -B build -DPICO_BOARD=pico2
cmake --build build
```

## Installer

Maintenir **BOOTSEL** pendant le branchement USB de la Pico 2, puis copier `build/water_gun.uf2` sur le volume qui apparaît.

## Simuler avec Wokwi

Compiler avec `cmake --build --preset pico2`, puis lancer **Wokwi: Start Simulator**
depuis la palette de commandes. Après une modification et une recompilation,
redémarrer la simulation pour charger le nouveau firmware.

Le moniteur série utilise UART0 (115200 bauds) : GP0 → RX et GP1 → TX,
reliés au moniteur virtuel dans `diagram.json`. Les `printf` ne passent pas par USB.
Sur la carte physique, ces messages nécessitent un adaptateur USB-UART.

Le programme balaie uniquement le servo 1 sur GP12 et affiche les cycles et les angles.
Les servos sur GP14 et GP16 ne sont pas encore commandés.
La LED intégrée clignote pendant le balayage et le moniteur série est toujours affiché.

Le calcul de la largeur des impulsions utilise des entiers : dans la simulation
testée, la version utilisant des instructions flottantes conservait une consigne
de 1000 µs même lorsque l'angle changeait. La version entière a été vérifiée dans
le débogueur Wokwi (36° → 1200 µs).
Le port GDB local est configuré sur 3333 dans `wokwi.toml`.
