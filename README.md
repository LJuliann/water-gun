<div align="center">

# Water Gun

Un projet de pistolet à eau piloté par Raspberry Pi Pico 2.

**C** &nbsp; / &nbsp; **RP2350** &nbsp; / &nbsp; **Pico SDK**

</div>

---

En développement : la base du firmware est en place ; le contrôle matériel reste à implémenter.

## Préparer ce poste (Linux x86_64)

Avec CMake ≥ 3.21, Ninja, Git, curl, tar, xz, Python 3 et un compilateur C++ natif
déjà installés, exécuter :

```sh
bash scripts/setup-env.sh
```

Le script installe Pico SDK 2.3.1 et ses sous-modules, Arm GNU Toolchain
15.2.Rel1 et picotool 2.3.1 dans `.tools/`, puis compile le firmware.
Il initialise aussi le sous-module `FreeRTOS-Kernel` à la version enregistrée dans le projet.
Les archives binaires sont vérifiées par SHA-256. Cette installation ne nécessite
pas `sudo` ; la première préparation nécessite Internet. CMake détecte ces outils
automatiquement. Les compilations suivantes fonctionnent hors ligne.

Pour VS Code, utiliser CMake Tools, clangd et Wokwi. Le fichier `.clangd` indique
la base de compilation `build/compile_commands.json`. Avec le compilateur local,
ajouter `--query-driver=${workspaceFolder}/.tools/arm-toolchain/bin/arm-none-eabi-*`
aux arguments de clangd dans `.vscode/settings.json`, puis lancer
**clangd: Restart language server** après la première configuration.

## Documenter les fonctions

Le projet utilise Doxygen et l'extension VS Code
[Doxygen Documentation Generator](https://github.com/cschlosser/doxdocgen).
Au-dessus d'une déclaration de fonction, saisir `/**` puis appuyer sur **Entrée**
pour créer un modèle avec résumé, paramètres et valeur de retour.
Compléter les descriptions et préciser `[in]`, `[out]` ou `[in,out]` selon le rôle des paramètres.

Pour générer la documentation depuis la racine du projet :

```sh
doxygen Doxyfile
```

Ouvrir ensuite `docs/doxygen/html/index.html` dans un navigateur. Dans VS Code,
la tâche **Doxygen : générer la documentation** exécute la même commande.
Les avertissements sont enregistrés dans `docs/doxygen/warnings.log`.
La génération est indépendante de la compilation du firmware.

Voir [le guide et les exemples de commentaires](docs/documentation.md).

## Compiler

Prérequis : CMake ≥ 3.21, une chaîne de compilation Arm embarquée et le Pico SDK avec ses sous-modules initialisés.

Après un clonage ou la récupération de l'ajout de FreeRTOS, initialiser le noyau :

```sh
git submodule update --init --recursive
```

`FreeRTOSConfig.h` contient la configuration du projet : un coeur, un tick de
1 ms, un tas de 32 Kio et les timers nécessaires à l'intégration avec le Pico SDK.
Les options ARM suivent les recommandations du
[port RP2350](https://github.com/raspberrypi/FreeRTOS-Kernel/blob/main/portable/ThirdParty/GCC/RP2350_ARM_NTZ/README.md).
Dans le code, inclure `FreeRTOS.h` avant `task.h` ; `projdefs.h` est déjà inclus
par `FreeRTOS.h` et n'a pas besoin d'être ajouté séparément.

Pour une installation système sur CachyOS / Arch Linux, au lieu du script local :

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

## Tester

Lancer les tests des utilitaires sur le PC avec :

```sh
bash scripts/test-utils.sh
```

Pour tester les commandes reçues dans le moniteur série Wokwi et utiliser le
débogueur, suivre le [guide de test](docs/testing.md).

## Installer

Maintenir **BOOTSEL** pendant le branchement USB de la Pico 2, puis copier `build/water_gun.uf2` sur le volume qui apparaît.

## Simuler avec Wokwi

Compiler avec `cmake --build --preset pico2`, puis lancer **Wokwi: Start Simulator**
depuis la palette de commandes. Après une modification et une recompilation,
redémarrer la simulation pour charger le nouveau firmware.

Si Wokwi demande une activation sur ce poste, lancer **Wokwi: Request a new License**
et suivre la [procédure officielle](https://docs.wokwi.com/vscode/getting-started).

Le moniteur série utilise UART0 (115200 bauds) : GP0 → RX et GP1 → TX,
reliés au moniteur virtuel dans `diagram.json`. Les `printf` ne passent pas par USB.
Sur la carte physique, ces messages nécessitent un adaptateur USB-UART.

Le programme utilise trois tâches : réception série, visée et tir. La dernière
consigne AIM en attente remplace la précédente dans une queue d'un élément ; elle
commande le pitch sur GP12 et le yaw sur GP16. Les tirs sont conservés dans une
queue séparée d'un élément : un seul tir peut attendre pendant un tir en cours,
les suivants sont rejetés tant que cette place est occupée. La pompe est activée
sur GP8 pendant 500–2000 ms avec GP9 à 0.
L'attente d'un tir laisse les autres tâches s'exécuter. Dans Wokwi, la LED verte
`sig1` représente l'activation de la pompe. Les quatre GPIO sont modifiables en
tête de `water_gun.c` ; voir le [câblage et les essais](docs/testing.md).

Le firmware est silencieux par défaut (`DEBUG=0`). Pour les traces de test,
configurer avec `-DWATER_GUN_DEBUG=ON`, puis compiler ; cette option est indépendante
des symboles de débogage. Les centres pitch/yaw sont configurables dans
`water_gun.c` et restent à calibrer avec les limites mécaniques avant utilisation
sur le montage réel.

Le preset `pico2` teste la réception stdio ; `pico2-uart-irq` active la réception
UART0 par interruptions. Le [guide de test](docs/testing.md) explique les deux modes.

Le calcul de la largeur des impulsions utilise des entiers : dans la simulation
testée, la version utilisant des instructions flottantes conservait une consigne
de 1000 µs même lorsque l'angle changeait. La version entière a été vérifiée dans
le débogueur Wokwi (36° → 1200 µs).
Le port GDB local est configuré sur 3333 dans `wokwi.toml`.
