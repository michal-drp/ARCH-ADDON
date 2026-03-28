import fontforge
import unicodedata

font = fontforge.activeFont()

for glyph in font.glyphs():
    # Získáme unicode hodnotu aktuálního glyfu
    uni_code = glyph.unicode
    
    # Pokud glyf nemá přiřazený unicode (je to např. speciální ligatura), přeskočíme ho
    if uni_code == -1:
        continue
    
    # Převedeme na charakter a zjistíme jeho velké "já"
    char = chr(uni_code)
    upper_char = char.upper()
    
    # Pokud se velké písmeno liší od původního (tzn. bylo to malé písmeno)
    if char != upper_char:
        try:
            # Najdeme, kde ve fontu leží to velké písmeno
            font.selection.select(upper_char)
            font.copy()
            
            # Vložíme ho na pozici původního malého písmena
            font.selection.select(glyph.encoding)
            font.paste()
            
            print(f"Změněno: {char} -> {upper_char}")
        except:
            # Pokud velké písmeno ve fontu neexistuje, nic neděláme
            continue

print("Hotovo. Všechna dostupná malá písmena byla nahrazena velkými.")