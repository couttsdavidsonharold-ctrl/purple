# PURPLE by Hdax - Installation (sans terminal)

## 1. Envoyer les fichiers sur GitHub (une seule fois)
1. Dezippe `PURPLE-by-Hdax-flat.zip` : tu obtiens des fichiers, AUCUN dossier.
2. Sur ton depot GitHub : **Add file > Upload files**, selectionne TOUS les fichiers (Cmd+A), glisse-les, **Commit changes**. (Les fichiers du meme nom sont remplaces.)
3. **Add file > Create new file**. Dans le nom, tape exactement `.github/workflows/build.yml` (chaque `/` cree un dossier). Ouvre `build.yml` du zip dans un editeur de texte, copie TOUT, colle-le dans la grande zone, puis **Commit changes**.
4. Onglet **Actions** : "Build PURPLE" demarre (15 a 25 min). Ensuite, onglet **Code > Releases** (colonne de droite) : tu trouves le `.exe` (Windows) et le `.pkg` (Mac).

## 2. Installer
Windows : double-clic sur le .exe ("Informations complementaires > Executer quand meme").
Mac : clic droit sur le .pkg > Ouvrir > Ouvrir (sinon Reglages Systeme > Confidentialite et securite > Ouvrir quand meme).
Puis relance ta DAW et fais un rescan des plugins : le plugin s'appelle **PURPLE by Hdax**.

AAX (Pro Tools) non inclus. Rien n'a encore ete teste dans une vraie DAW.
