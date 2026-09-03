#!/usr/bin/env python3
"""Draw src/app/albert.ico -- the caricature the emulator is named after.

The icon is generated rather than pasted in so it can be re-cut at any size and
so the shapes stay editable. Everything is laid out in a 0..1 unit square and
scaled to the target, then supersampled 4x and reduced, which is what keeps the
edges clean at 16 pixels.

Run it from anywhere:  python tools/make_icon.py
"""

import os
from PIL import Image, ImageDraw, ImageFilter

# The whole design is a silhouette test: at 16 pixels all that survives is a
# dark disc, a white flare of hair and a white bar of moustache. Every colour
# here is chosen to keep those three apart.
BG        = (26, 31, 42, 255)     # dark slate, the same family as the app's UI
BG_RIM    = (58, 69, 90, 255)
SHOULDER  = (52, 62, 82, 255)
HAIR      = (240, 242, 238, 255)
HAIR_DK   = (196, 201, 203, 255)
SKIN      = (228, 178, 132, 255)
SKIN_DK   = (196, 146, 102, 255)
INK       = (38, 33, 28, 255)


def draw(px):
    """Render the icon at px by px, supersampled."""
    ss = 4
    n = px * ss
    im = Image.new("RGBA", (n, n), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    def box(x0, y0, x1, y1):
        return [x0 * n, y0 * n, x1 * n, y1 * n]

    def ell(b, fill):
        d.ellipse(box(*b), fill=fill)

    # Ground: a disc rather than a full square, so the icon reads as a badge at
    # any size and keeps its shape against a light or a dark taskbar.
    ell((0.015, 0.015, 0.985, 0.985), BG_RIM)
    ell((0.035, 0.035, 0.965, 0.965), BG)

    # Shoulders, clipped by the disc further down. Dark enough to stay
    # background, light enough to stop the head floating.
    sh = Image.new("RGBA", (n, n), (0, 0, 0, 0))
    ImageDraw.Draw(sh).ellipse(box(0.14, 0.86, 0.86, 1.26), fill=SHOULDER)
    mask = Image.new("L", (n, n), 0)
    ImageDraw.Draw(mask).ellipse(box(0.035, 0.035, 0.965, 0.965), fill=255)
    im.paste(sh, (0, 0), Image.composite(mask, Image.new("L", (n, n), 0), sh.split()[3]))

    # Ears first, so the head covers where they join.
    ell((0.255, 0.545, 0.325, 0.665), SKIN_DK)
    ell((0.675, 0.545, 0.745, 0.665), SKIN_DK)

    # Head, a touch narrower than it is tall. The high forehead is half the
    # likeness: the crown hair stops around 0.39 and leaves skull showing from
    # there down to the brows.
    ell((0.300, 0.315, 0.700, 0.865), SKIN)

    # Hair: overlapping puffs, no spikes. The mass sits low and wide, flaring
    # outward at ear level rather than piling up on top -- that, plus the high
    # hairline, is what separates Einstein from a judge's wig. Deliberately not
    # symmetric; a mirrored cloud reads as a wig too.
    #
    # Ragged spikes were tried and taken out again. They wanted to be longer
    # than the mass to show at all, which meant they only sprouted where the
    # outline was thin -- a pair of horns at the temples, and a row of them
    # along the crown that read as a tiara.
    for b in ((0.088, 0.420, 0.298, 0.626),
              (0.112, 0.318, 0.335, 0.532),
              (0.166, 0.256, 0.368, 0.432),
              (0.702, 0.432, 0.912, 0.632),
              (0.665, 0.310, 0.888, 0.520),
              (0.632, 0.264, 0.834, 0.432)):
        ell(b, HAIR)
    ell((0.330, 0.250, 0.672, 0.392), HAIR)   # thin crown band, high hairline
    ell((0.252, 0.282, 0.436, 0.446), HAIR)   # bridges across the temples, so
    ell((0.572, 0.288, 0.752, 0.452), HAIR)   # the outline has no notch in it
    # A little shading where the flares pass behind the head.
    ell((0.190, 0.495, 0.300, 0.600), HAIR_DK)
    ell((0.700, 0.495, 0.810, 0.600), HAIR_DK)

    # Brows, heavy and white, tilted out to the sides.
    for x0, x1, y0, y1 in ((0.338, 0.470, 0.500, 0.474),
                           (0.530, 0.662, 0.474, 0.500)):
        d.line([(x0 * n, y0 * n), (x1 * n, y1 * n)], fill=HAIR, width=int(0.048 * n))

    # Eyes. Kept small and dark: at 16 pixels they become two grey pixels,
    # which is all a face at that size needs.
    ell((0.378, 0.552, 0.448, 0.615), INK)
    ell((0.552, 0.552, 0.622, 0.615), INK)

    # Nose, as shading only.
    ell((0.462, 0.590, 0.538, 0.700), SKIN_DK)

    # The moustache: one lozenge and two droops, the second silhouette cue.
    ell((0.352, 0.672, 0.648, 0.762), HAIR)
    ell((0.330, 0.668, 0.462, 0.792), HAIR)
    ell((0.538, 0.668, 0.670, 0.792), HAIR)

    out = im.resize((px, px), Image.LANCZOS)
    # Downsampling to 16 or 24 costs the edges more than it costs the shapes.
    if px <= 32:
        out = out.filter(ImageFilter.UnsharpMask(radius=1.0, percent=70, threshold=0))
    return out


SIZES = [16, 20, 24, 32, 48, 64, 128, 256]


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.dirname(here)
    imgs = [draw(s) for s in SIZES]
    ico = os.path.join(root, "src", "app", "albert.ico")
    imgs[-1].save(ico, format="ICO",
                  sizes=[(s, s) for s in SIZES], append_images=imgs[:-1])
    print("wrote", ico)


if __name__ == "__main__":
    main()
