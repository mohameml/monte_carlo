# **Les étapes pour configurer VS Code pour le développement en C++ avec CMake:**

> Voici un guide détaillé sur la façon de configurer Visual Studio Code (VS Code) pour le développement en C++ avec CMake, en incluant l'installation des extensions, la création de fichiers de configuration, et la configuration des chemins vers des bibliothèques externes comme PNL.

### 1. **Étape 1 : Installation des Extensions C++ et CMake Tools**

1. **Ouvrez Visual Studio Code**.

2. **Accédez à l'onglet Extensions** :

    - Cliquez sur l'icône Extensions (ou appuyez sur `Ctrl + Shift + X`).

3. **Recherchez et installez les extensions suivantes** :

    - **C/C++** par Microsoft : Cette extension fournit des fonctionnalités de support pour le développement C et C++, y compris l'auto-complétion, le débogage et la navigation dans le code.

    - **CMake Tools** par Microsoft : Cette extension facilite l'utilisation de CMake dans VS Code, en fournissant des commandes pour configurer, construire et déboguer des projets CMake.

### 2. **Étape 2 : Création d'un Projet CMake avec `CMake: Quick Start`**

1. **Ouvrez la palette de commandes** :

    - Appuyez sur `Ctrl + Shift + P`.

2. **Tapez "CMake: Quick Start"** et sélectionnez-le :

    - Cela créera une structure de projet CMake de base, y compris un fichier `CMakeLists.txt`.

3. **Suivez les instructions** :
    - Donnez un nom à votre projet et choisissez un répertoire de destination. Cela générera le fichier `CMakeLists.txt` dans le répertoire choisi.

### 3. **Étape 3 : Configuration de CMakePresets**

1. **Créer un fichier `CMakePresets.json`** :
    - À la racine de votre projet, créez un fichier nommé `CMakePresets.json` s'il n'existe pas déjà.
2. **Ajouter la configuration** :
    - Ajoutez les chemins vers les bibliothèques que vous utilisez, par exemple PNL, en utilisant le format suivant :

```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "default",
            "hidden": false,
            "generator": "Ninja",
            "cacheVariables": {
                "CMAKE_PREFIX_PATH": "/chemin/vers/votre/bibliotheque/pnl/build",
                "CMAKE_BUILD_TYPE": "Debug"
            }
        }
    ]
}
```

### 4. **Étape 4 : Configuration de `c_cpp_properties.json`**

1. **Ouvrez la palette de commandes** :

    - Appuyez sur `Ctrl + Shift + P`.

2. **Tapez "C/C++: Edit Configurations (UI)"** et sélectionnez-le :

    - Cela ouvrira le fichier `c_cpp_properties.json`.

3. **Ajoutez le chemin vers les fichiers d'en-tête de PNL** :

    - Dans la section `includePath`, ajoutez le chemin vers le répertoire d'inclusion de PNL :

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "/chemin/vers/votre/bibliotheque/pnl/build/include"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
```

### 5. **Étape 5 : Configuration de `launch.json`**

1. **Ouvrez la palette de commandes** :

    - Appuyez sur `Ctrl + Shift + P`.

2. **Tapez "Debug: Open launch.json"** et sélectionnez-le :

    - Si aucun fichier `launch.json` n'existe, VS Code vous demandera de choisir un environnement. Sélectionnez C++ (GDB/LLDB).

3. **Modifiez le fichier pour utiliser CMake et ajustez le chemin du programme** :

    - Remplacez le contenu de `launch.json` par la configuration suivante :

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "C/C++: gcc build and debug active file",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/out/build/${fileBasenameNoExtension}", // Chemin de l'exécutable
            "args": [],
            "stopAtEntry": false,
            "cwd": "${fileDirname}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description": "Set Disassembly Flavor to Intel",
                    "text": "-gdb-set disassembly-flavor intel",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "CMake: Build", // Assurez-vous que cette tâche est définie dans tasks.json
            "miDebuggerPath": "/usr/bin/gdb"
        }
    ]
}
```

### 6. **Étape 6 : Configuration de `tasks.json`**

1. **Ouvrez la palette de commandes** :

    - Appuyez sur `Ctrl + Shift + P`.

2. **Tapez "Tasks: Configure Default Build Task"** et sélectionnez-le :

    - Si aucun fichier `tasks.json` n'existe, VS Code vous demandera de choisir un type de tâche. Sélectionnez CMake.

3. **Ajoutez la configuration pour CMake** :

    - Le fichier `tasks.json` pourrait ressembler à ceci :

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake: Configure", // Nom de la tâche
            "type": "shell",
            "command": "cmake", // Commande pour exécuter CMake
            "args": [
                "-S",
                "${workspaceFolder}", // Dossier source
                "-B",
                "${workspaceFolder}/out/build" // Dossier de sortie pour les fichiers de construction
            ],
            "group": {
                "kind": "build",
                "isDefault": true // Cette tâche sera utilisée comme tâche de construction par défaut
            },
            "problemMatcher": ["$gcc"] // Modèle pour détecter les erreurs de compilation
        },
        {
            "label": "CMake: Build", // Nom de la tâche de construction
            "type": "shell",
            "command": "cmake", // Commande pour exécuter CMake
            "args": [
                "--build",
                "${workspaceFolder}/out/build" // Dossier de sortie pour la construction
            ],
            "group": {
                "kind": "build",
                "isDefault": false // Pas une tâche de construction par défaut
            },
            "problemMatcher": ["$gcc"] // Modèle pour détecter les erreurs de compilation
        }
    ]
}
```

### 7. **Étape 7 : Vérification et Exécution**

1. **Vérifiez que toutes les configurations sont correctes** :

    - Assurez-vous que les chemins vers les bibliothèques et les exécutables sont corrects.

2. **Construisez le projet** :

    - `Ctrl + Shift + P` : puis sélectionner `CMake:Build`

3. **Déboguez l'application** :

    - Ouvrez le fichier source que vous souhaitez déboguer, puis lancez le débogage en appuyant sur `F5`.
