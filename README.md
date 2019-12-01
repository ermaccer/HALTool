# HALTool
Create and extract HAL archives used in ICEngine games.

# Usage

To extract

`haltool -e -o output_folder main.hal`

In order to create an archive, you need a list file to save the order (game expects certain files on certain places).

`haltool -e -o output_folder -l listname.txt main.hal`

This will create a list file listname.txt.

To create

`haltool -c -o newmain.hal -l listname.txt foldername`
