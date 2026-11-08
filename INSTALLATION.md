# PURPLE by Hdax — Installation (sans terminal)

Tu n'as JAMAIS besoin du terminal. Le terminal sert uniquement à compiler le plugin soi-même.
Ici, c'est GitHub qui compile pour toi (gratuit), et tu télécharges un installateur normal.

## Étape 1 — Obtenir les installateurs (une seule fois, ~15 min)
1. Crée un compte gratuit sur github.com, puis clique sur **+ > New repository**. Nom : `purple`. Coche **Private**. Clique **Create repository**.
2. Dézippe `PURPLE-by-Hdax-source.zip`. Sur la page du dépôt, clique **uploading an existing file** et glisse TOUT le contenu du dossier `purple`.
   - **Important** : le dossier `.github` est caché sur Mac (Finder : appuie sur **Cmd + Maj + .** pour l'afficher) et sur Windows (Explorateur : Affichage > Afficher > Éléments masqués).
   - Si tu n'arrives pas à glisser `.github` : clique **Add file > Create new file**, tape comme nom `.github/workflows/build.yml`, puis colle le contenu du fichier `COPIE-VISIBLE-DU-WORKFLOW/build.yml`.
3. Clique **Commit changes**.
4. Ouvre l'onglet **Actions** : « Build PURPLE » démarre tout seul (environ 15 à 25 min).
5. Quand c'est vert, clique **Releases** (colonne de droite de la page du dépôt) : tu y trouves
   - `PURPLE-by-Hdax-0.1.0-Windows-Setup.exe`
   - `PURPLE-by-Hdax-0.1.0-macOS.pkg`
   Si le dépôt est privé, tu dois être connecté pour télécharger.
6. Si un rond rouge apparaît : ouvre-le, copie le message d'erreur et envoie-le-moi, je corrige.

## Étape 2 — Installer
**Windows** : double-clic sur le `.exe`. Si « Windows a protégé votre ordinateur » : **Informations complémentaires > Exécuter quand même** (l'installateur n'est pas signé).
**Mac** : clic droit sur le `.pkg` > **Ouvrir** > **Ouvrir**. Si Mac refuse : Réglages Système > Confidentialité et sécurité > **Ouvrir quand même**.

Emplacements : VST3 `C:\Program Files\Common Files\VST3` (Windows) ou `/Library/Audio/Plug-Ins/VST3` (Mac) ; AU (Mac) `/Library/Audio/Plug-Ins/Components`.

## Étape 3 — Dans la DAW
Relance la DAW et force un nouveau scan des plugins (FL Studio : Options > Manage plugins > Find installed plugins ; Ableton : Préférences > Plug-ins > Rescan ; Logic : il scanne seul au lancement ; Reaper : Preferences > Plug-ins > VST > Re-scan).
Le plugin apparaît sous le nom **PURPLE by Hdax** (fabricant : Hdax).

## À savoir
- Pro Tools (AAX) n'est pas inclus : il demande le SDK Avid et une signature officielle.
- Ces installateurs n'ont pas encore été testés par moi sur Windows/Mac ni dans une DAW.
