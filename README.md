#Compile:

cd engine

make OS=WINDOWS


#Example usage:

.\simc.exe Shaman_T10.simc html=Shaman_T10.html

.\simc.exe Mage_T9.simc html=Segv.html

.\simc.exe Mage_T9.simc > mage.txt

## Not working with wotlk version:

.\simc.exe wowhead=14320165 save=Paladin_T9_05_11_55.simc

.\simc.exe armory=us,Llane,Segv save=Segv.simc

.\simc.exe armory=us,illidan,john calculate_scale_factors=1 html=john.html

Also see READ_ME_FIRST.txt