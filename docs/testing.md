# Tester le firmware et les commandes série

## Tests des utilitaires sur le PC

Depuis la racine du projet :

```sh
bash scripts/test-utils.sh
```

Ce script compile directement `gun_utils.c` avec le compilateur du PC, puis lance
trois suites CTest : conversion des entiers, lecture jusqu'au délimiteur et
décodage des commandes. Les tests couvrent les limites des valeurs, les fins de
ligne LF/CRLF, les entrées invalides et la conservation des sorties en cas d'échec.
Ils n'utilisent pas le SDK Pico et n'exigent pas de carte ni de simulateur.

Dans VS Code, lancer **Tasks: Run Task** puis **Tests : utilitaires sur le PC**.
La réussite est indiquée par `100% tests passed` pour les trois suites.

## Tester la réception dans Wokwi

1. Pour cet essai avec les messages de diagnostic, compiler avec :

   ```sh
   cmake --preset pico2 -DWATER_GUN_DEBUG=ON
   cmake --build --preset pico2
   ```

2. Lancer **Wokwi: Start Simulator** depuis la palette **Ctrl+Maj+P**.
3. Dans le panneau **Terminal** de VS Code, sélectionner **Wokwi Terminal** et
   attendre `Ready: serial -> aim/shoot (stdio)`.
4. Cliquer dans ce terminal, taper ou coller une commande puis **Entrée**.

L'extension Wokwi de ce poste transmet la saisie directement au microcontrôleur
sans afficher les caractères tapés (pas d'écho local). Il est donc normal de ne
pas voir la commande pendant la saisie ; la réponse du firmware apparaît après
**Entrée**. Ne pas saisir les deux caractères `\` et `n` : **Entrée** transmet
la fin de ligne. Le firmware accepte CR, LF et CRLF pour terminer une ligne.
Garder l'onglet du simulateur visible et la simulation en cours pendant l'essai.

| Saisie dans le moniteur | Réponse attendue |
| --- | --- |
| `<pitch10;yaw-20>` | `AIM: pitch=100 yaw=-200 (0.1 deg)` |
| `<pitch13.4;yaw-7.6>` | `AIM: pitch=134 yaw=-76 (0.1 deg)` |
| `<pitch-90;yaw90>` | `AIM: pitch=-900 yaw=900 (0.1 deg)` |
| `<shoot500>` | `SHOOT: duration=500 ms`, puis `SHOOT: done` |
| `<shoot750>` | `SHOOT: duration=750 ms`, puis `SHOOT: done` |
| `<shoot2000>` | `SHOOT: duration=2000 ms`, puis `SHOOT: done` |
| `<shoot3000>` | `Invalid command` |
| `<pitch75.1;yaw0>` | `Invalid command` |
| `<pitch13.45;yaw0>` | `Invalid command` |
| `<shoot750>extra` | `Invalid command` |

Pour vérifier la récupération après saturation, envoyer une ligne contenant au
moins 64 caractères `x`, puis **Entrée** : le firmware doit afficher
`Serial line discarded`. Envoyer ensuite `<shoot750>` : la réponse doit à nouveau
être `SHOOT: duration=750 ms`, puis `SHOOT: done`.

Une ligne vide est ignorée. Les commandes doivent être séparées par des fins de
ligne. Après une recompilation, redémarrer la simulation pour charger le nouveau
firmware.

Les presets compilent avec `DEBUG=0` par défaut, y compris en configuration Debug :
le firmware reste débogable, mais ses traces `printf` ne sont pas compilées.
L'option CMake `-DWATER_GUN_DEBUG=ON` définit `DEBUG=1` pour activer les messages
montrés dans ce guide. Tous sont protégés par `#if defined(DEBUG) && DEBUG`.
**Ctrl+Maj+B** reconfigure le preset silencieux ; pour conserver les traces d'un
essai, utiliser `cmake --build build` après la configuration explicite ci-dessus.
`task_serial` décode et distribue les commandes. La queue de `task_aim` contient
un seul élément : `xQueueOverwrite` remplace toute position encore en attente par
la plus récente. La queue de `task_shoot` sert uniquement à transmettre un tir
accepté. Dès l'acceptation et jusqu'à la fin du traitement, toutes les autres
demandes SHOOT sont rejetées, même si la queue est déjà vide. Avec les traces
activées, le message est `SHOOT: busy`. Aucun tir n'est conservé pour plus tard.
Une nouvelle demande peut être acceptée après la fin du précédent tir.

## GPIO et contrôle des actionneurs

Les quatre GPIO sont regroupés en tête de `water_gun.c` pour adapter facilement
le firmware au câblage de la carte. Les valeurs par défaut reprennent le firmware
V1 indiqué et les connexions présentes dans `diagram.json` :

| Fonction | Constante | GPIO | Élément Wokwi |
| --- | --- | --- | --- |
| Servo pitch | `PITCH_SERVO_PIN` | GP16 | `servo3` |
| Servo yaw | `YAW_SERVO_PIN` | GP12 | `servo1` |
| Pompe, activation | `PUMP_EN_PIN` | GP8 | LED verte `sig1`, U2 EN/IN1 |
| Pompe, direction | `PUMP_PH_PIN` | GP9 | LED jaune `sig2`, U2 PH/IN2 |

Les constantes `PITCH_SERVO_CENTER_DEG` et `YAW_SERVO_CENTER_DEG` définissent les
centres, au démarrage et dans la conversion des commandes : canon horizontal et
dans l'axe du drone. Mesurés sur le drone : pitch 77° (le canon était à
l'horizontale avec `<pitch13;yaw0>` et un centre à 90°), yaw 90°.
`PITCH_SERVO_DIRECTION` et `YAW_SERVO_DIRECTION` (+1 ou −1)
alignent le sens des servos sur le repère du drone utilisé par le Jetson
(REP-103) : pitch positif = le canon monte, yaw positif = il tourne vers la
gauche du drone, vu de derrière le canon. Sur le drone, les deux valent −1 :
`<pitch0;yaw0>` donne 77° (pitch) et 90° (yaw), et `<pitch10;yaw-20>` donne
respectivement 67° et 110°. La calibration PWM reste celle du projet
(1000–2000 µs).

Les angles acceptent un dixième de degré (`<pitch13.4;yaw-7.6>`) ; les degrés
entiers restent valides. Ils sont conservés en dixièmes jusqu'au PWM, dont le pas
de 1 µs vaut 0,18° : un pas de 1° (environ 5 cm à 3 m) faisait sauter la visée.
Limites : pitch de −90 à 75° (le servo est en butée à 77°), yaw ±90°.

La V1 `gun_a_eau.c` utilise des offsets pitch −60° et yaw +70°, mais reçoit des
angles de 0 à 180°, contrairement au protocole signé actuel. Ces offsets ne sont
donc pas recopiés automatiquement. Les centres, le sens et les limites mécaniques
doivent être calibrés avant les mouvements sur le montage réel. La limitation
logicielle générique de 0 à 180° dans `set_servo_angle` ne définit pas ces limites
mécaniques.

La pompe démarre désactivée. Chaque tir maintient PH à 0, active EN pendant la
durée demandée, puis remet EN à 0. Dans Wokwi, `sig1` indique cette activation ;
`sig2` reste éteinte. GP14, GP10/GP11 et le moteur pas à pas ne sont pas commandés.

Avant d'activer EN, `task_shoot` arme une alarme du Pico SDK pour la durée du tir.
Son callback `pump_cutoff` remet directement EN à 0 depuis une interruption,
indépendamment du réveil de la tâche FreeRTOS. Si la durée est hors de 500–2000 ms
ou si l'alarme ne peut pas être armée, la pompe reste arrêtée. Le chemin normal
remet aussi EN à 0 et annule l'alarme avant d'accepter un nouveau tir.

Cette protection ne nécessite aucune nouvelle commande ROS2. Elle borne la durée
du tir sans attendre d'autres données ; elle ne détecte pas une déconnexion et
n'arrête pas immédiatement un tir lors de celle-ci. La coupure dépend encore du
timer et de son interruption : elle ne couvre pas un blocage global avec les
interruptions désactivées. Pour les essais physiques, une entrée EN maintenue à
l'arrêt pendant un reset et une coupure matérielle adaptée restent à prévoir.
Voir le [contrat des alarmes du Pico SDK](https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#group_alarm).

Pour vérifier l'indépendance des tâches, envoyer `<shoot2000>`, puis envoyer
`<pitch25;yaw90>` avant l'apparition de `SHOOT: done`. Les servos doivent changer
de position alors que la LED verte de la pompe est encore allumée. La tâche de tir
attend avec `vTaskDelay`, ce qui laisse la réception et la visée s'exécuter.
Envoyer aussi `<shoot500>` pendant ce tir : la réponse doit être `SHOOT: busy`
et la LED ne doit pas se rallumer automatiquement après les deux secondes.
Une fois `SHOOT: done` affiché, renvoyer `<shoot500>` : il doit être accepté.

Ne pas utiliser un arrêt GDB comme preuve de la coupure de sécurité : si le
débogueur arrête le cœur, le callback d'interruption ne peut plus s'exécuter.

Pour vérifier la dernière position en attente, coller plusieurs commandes AIM
séparées par des fins de ligne. La position finale doit correspondre à la dernière
commande valide. Des positions intermédiaires peuvent être appliquées si la tâche
de visée a le temps de s'exécuter entre les réceptions.

## Tester la réception UART avec interruptions

Le preset par défaut `pico2` utilise `getchar_timeout_us` avec `USE_UART_IRQ=0`.
Pour compiler la branche qui utilise les interruptions RX d'UART0 et un stream
buffer FreeRTOS :

```sh
cmake --preset pico2-uart-irq -DWATER_GUN_DEBUG=ON
cmake --build --preset pico2-uart-irq
```

Dans VS Code, la tâche **Pico 2 : compiler UART IRQ** exécute cette configuration.
Redémarrer Wokwi et attendre `Ready: serial -> aim/shoot (UART0 IRQ)`.
Les commandes et les réponses attendues sont les mêmes que dans le tableau.
Le câblage reste GP0 vers le RX du moniteur et GP1 vers son TX, à 115200 bauds.

Les deux presets produisent le firmware au même emplacement `build/water_gun.uf2`
pour Wokwi. Reconfigurer avec le preset choisi avant de compiler. **Ctrl+Maj+B**
reconfigure le mode par défaut `pico2`, sans interruptions RX.

## Observer le décodage avec le débogueur

1. Compiler le firmware.
2. Placer un point d'arrêt dans `decode_command` dans `gun_utils.c`, ou dans
   `task_aim` ou `task_shoot` juste après la réception de leur queue.
3. Lancer **Wokwi: Start Simulator and Wait for Debugger**.
4. Choisir **Wokwi : déboguer le firmware** pour le mode stdio ou
   **Wokwi : déboguer UART IRQ** pour le mode interruptions, puis appuyer sur **F5**.
5. Continuer avec **F5**, puis envoyer une commande dans le moniteur série.

Le débogueur Cortex-Debug utilise le GDB Arm local et le serveur Wokwi sur
`127.0.0.1:3333`. Il permet d'observer `input`, `cursor`, `temp` et `command`.
Le simulateur doit être lancé séparément avant la connexion du débogueur.

Les réglages de VS Code sont locaux dans `.vscode/`. Les sources des tests,
le script et ce guide peuvent être versionnés ; les binaires restent dans `build/`.

Références : [moniteur série Wokwi](https://docs.wokwi.com/guides/serial-monitor),
[débogage Wokwi](https://docs.wokwi.com/vscode/debugging) et
[serveur GDB externe Cortex-Debug](https://github.com/Marus/cortex-debug/wiki/External-gdb-server-configuration).
