# HALTool
Create and extract HAL archives used in ICEngine games.

# Supported games
- Tytus, Romek i A'Tomek (Monkeys Adventrues)
- Wilczy Szaniec
- Tajemnicza Wyspa
- Mortyr
- Warhammer 40,000: Agents Of Death

# Usage

Few games require -m switch, keep that in mind.

To extract

`haltool -e -o output_folder main.hal`

In order to create an archive, you need a list file to save the order (game expects certain files on certain places).

`haltool -e -o output_folder -l listname.txt main.hal`

This will create a list file listname.txt.

To create

`haltool -c -o newmain.hal -l listname.txt foldername`
