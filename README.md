# Water Gun

Firmware **Raspberry Pi Pico 2 / RP2350**, avec FreeRTOS.

## Préparer

Linux x86_64 : CMake ≥ 3.21, Ninja, Git, curl, tar, xz, Python 3 et g++ requis.

```sh
bash scripts/setup-env.sh
```

Autres systèmes : installer Pico SDK 2.3.1 avec ses sous-modules et un compilateur Arm `arm-none-eabi`, puis définir `PICO_SDK_PATH`.

## Choisir le firmware

Exécuter **une** commande de configuration, puis compiler :

| Version | Commande de configuration | UF2 fourni dans le ZIP (`firmware/`) |
| --- | --- | --- |
| Stdio, silencieux | `cmake --preset pico2` | `pico2-stdio.uf2` |
| Stdio, traces de test | `cmake --preset pico2 -DWATER_GUN_DEBUG=ON` | `pico2-stdio-test.uf2` |
| UART avec interruptions, silencieux | `cmake --preset pico2-uart-irq` | `pico2-uart-irq.uf2` |
| UART avec interruptions, traces de test | `cmake --preset pico2-uart-irq -DWATER_GUN_DEBUG=ON` | `pico2-uart-irq-test.uf2` |
| **Pico 1 (RP2040), USB** : drone (Jetson, `/dev/ttyACM0`) | `cmake --preset pico-usb` puis `cmake --build build-pico` | — |

```sh
cmake --build build
```

Résultat : `build/water_gun.uf2` et `build/water_gun.elf`. `Debug` conserve les symboles du débogueur ; seuls les modes « traces de test » activent les `printf`.

## Utiliser

- **Carte** : maintenir BOOTSEL au branchement USB, puis copier le UF2 choisi sur le volume de la Pico 2.
- **Wokwi** : lancer **Wokwi: Start Simulator**. Le ZIP fournit déjà `build/` avec la version UART IRQ de test. Après compilation, redémarrer la simulation.
- **Commandes** : dans **Wokwi Terminal**, envoyer `<pitch10;yaw-20>`, `<pitch13.4;yaw-7.6>` (dixièmes de degré) ou `<shoot750>`, puis Entrée. La saisie n'est pas affichée.

Communication UART0 à **115200 bauds**, **GP0 TX / GP1 RX** ; le stdio USB est désactivé.
Exception : `pico-usb` reçoit les commandes par l'USB (port série `/dev/ttyACM0` sur le Jetson) et n'utilise pas l'UART.
GPIO : pitch **16**, yaw **12**, pompe EN **8** / PH **9**. GPIO et centres réglables dans `water_gun.c` ; calibrer le montage avant utilisation réelle.
