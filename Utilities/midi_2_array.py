import math
import pretty_midi

# Load the MIDI file (use a single-track/monophonic MIDI for best results)
midi_data = pretty_midi.PrettyMIDI('/Users/degree/Desktop/midis/Pokemon Red, Blue, Yellow - Wild Pokemon Battle Theme.mid')

frequencies = []
durations = []

# Extract notes from the first instrument/track
for note in midi_data.instruments[0].notes:
    # Convert MIDI pitch to frequency (Hz)
    freq = int(440.0 * (2.0 ** ((note.pitch - 69.0) / 12.0)))
    # Calculate duration in milliseconds
    duration_ms = int((note.end - note.start) * 1000)
    
    frequencies.append(round(freq, 2))
    durations.append(duration_ms)

# Generate C array code
c_code = f"""// Auto-generated MIDI to C arrays
#ifndef MELODY_DATA_H
#define MELODY_DATA_H

#define MELODY_LENGTH {len(frequencies)}

static const uint16_t melody_freqs[MELODY_LENGTH] = {{
    {', '.join(map(str, frequencies))}
}};

static const uint16_t melody_durations[MELODY_LENGTH] = {{
    {', '.join(map(str, durations))}
}};

#endif // MELODY_DATA_H
"""

with open("melody_data.h", "w") as f:
    f.write(c_code)

print("Generated melody_data.h successfully!")
