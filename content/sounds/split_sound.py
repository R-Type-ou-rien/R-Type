import numpy as np
from scipy.io import wavfile

def create_clean_loop(filename, output_intro, output_loop):
    samplerate, data = wavfile.read(filename)

    # PARAMÈTRES
    ignore_end_sec = 0.5  # On jette les 0.5 dernières secondes (la partie qui "coupe")
    loop_duration_sec = 0.8  # On prend 0.8 seconde de son stable pour la boucle
    fade_duration_sec = 0.1 # Durée du lissage (crossfade)

    # Conversion en nombre d'échantillons
    n_ignore = int(ignore_end_sec * samplerate)
    n_loop = int(loop_duration_sec * samplerate)
    n_fade = int(fade_duration_sec * samplerate)

    # 1. On définit la zone utile (Tout le fichier MOINS la fin bizarre)
    useful_data = data[:-n_ignore]

    # 2. On extrait la base de la boucle (la fin de la zone utile)
    raw_loop = useful_data[-n_loop:]

    # 3. Création de l'Intro (Tout ce qui est avant la boucle)
    # L'intro va du début jusqu'au point où la boucle commence
    split_index = len(useful_data) - n_loop
    intro_data = useful_data[:split_index + n_fade] # On garde un peu de marge pour le chevauchement si besoin

    # 4. Le Crossfade (Lissage) pour rendre la boucle parfaite
    fade_out_part = raw_loop[-n_fade:]
    fade_in_part = raw_loop[:n_fade]

    fade_out_curve = np.linspace(1.0, 0.0, n_fade)
    fade_in_curve = np.linspace(0.0, 1.0, n_fade)

    mixed_start = (fade_in_part * fade_in_curve) + (fade_out_part * fade_out_curve)

    # Reconstitution de la boucle finale
    final_loop = np.concatenate((mixed_start, raw_loop[n_fade:-n_fade]))

    # Sauvegarde
    wavfile.write(output_intro, samplerate, intro_data)
    wavfile.write(output_loop, samplerate, final_loop.astype(data.dtype))
    print("Fichiers générés : Intro propre et Boucle sans la fin coupée.")

# Exécution
create_clean_loop("is_charging.wav", "charge_intro_v3.wav", "charge_loop_v3.wav")