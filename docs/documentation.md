# Documenter une nouvelle fonction

Documenter les fonctions publiques au-dessus de leur déclaration dans le fichier
`.h`. Pour une fonction interne, placer le commentaire au-dessus de sa définition
dans le fichier `.c`. Éviter de recopier la même description aux deux endroits.

Dans VS Code, l'extension Doxygen Documentation Generator crée le modèle lorsque
tu saisis `/**` au-dessus de la fonction puis appuies sur **Entrée**. Le résumé
et les explications restent à remplir. Si l'extension vient d'être installée et
ne réagit pas, lancer **Developer: Reload Window**.

## Exemple pour une conversion d'entier

```c
/**
 * @brief Convertit une chaîne décimale en entier signé.
 *
 * @param[in] text Chaîne terminée par '\0', avec un éventuel signe '-'.
 * @param[out] value Reçoit l'entier en cas de succès ; inchangé en cas d'échec.
 *
 * @return true si la conversion réussit.
 * @return false si un pointeur est nul, si la chaîne est vide ou invalide,
 *         ou si la valeur dépasse les limites d'un int.
 */
bool parse_integer(const char *text, int *value);
```

## Paramètres d'entrée et de sortie

- `[in]` : valeur ou données lues par la fonction.
- `[out]` : résultat écrit par la fonction.
- `[in,out]` : valeur reçue puis mise à jour.

Pour `parse_until`, `cursor` est `[in,out]` : après une conversion réussie,
il pointe sur le caractère qui suit le délimiteur. `delimiter` est `[in]` et
`value` est `[out]`. En cas d'échec, `cursor` et `value` restent inchangés.

Préciser les unités, les valeurs acceptées et les cas d'échec dans les commentaires.
Pour une fonction `void`, ne pas ajouter de balise `@return`.

## Générer et consulter le résultat

Depuis la racine du dépôt :

```sh
doxygen Doxyfile
xdg-open docs/doxygen/html/index.html
```

Ou utiliser **Tasks: Run Task** puis **Doxygen : générer la documentation** dans
la palette de commandes de VS Code.

La configuration couvre les fichiers `.c`, `.h` et `.md` à la racine du projet
ainsi que ce guide. Pour ajouter un sous-dossier de sources, l'ajouter à `INPUT`
dans `Doxyfile`. Les dépendances Pico SDK et FreeRTOS ne sont pas parcourues.

`EXTRACT_ALL = YES` permet de voir aussi les fonctions encore non documentées ;
leur présence dans la liste ne signifie pas que leur description est complète.
Les fichiers sont générés dans `docs/doxygen/` et ignorés par Git. Les anciens
répertoires `html/` et `latex/` sont également ignorés ; la configuration actuelle
produit uniquement une documentation HTML.

Références : [Doxygen](https://www.doxygen.nl/manual/config.html) et
[Doxygen Documentation Generator](https://github.com/cschlosser/doxdocgen).
