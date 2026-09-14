package net.meowsers.peach.structures;

import java.util.Objects;

public class Color {
    public float r, g, b, a;

    public Color(float n) {
        this(n, n, n, 1.f);
    }

    public Color(float r, float g, float b) {
        this(r, g, b, 1.f);
    }

    public Color(float r, float g, float b, float a) {
        this.r = r;
        this.g = g;
        this.b = b;
        this.a = a;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        Color otherColor = (Color) obj;
        return r == otherColor.r
                && g == otherColor.g
                && b == otherColor.b
                && a == otherColor.a;
    }

    @Override
    public int hashCode() {
        return Objects.hash(r, g, b, a);
    }

    // Core Primaries & Basics
    public static final Color White       = new Color(1.0f);
    public static final Color Black       = new Color(0.0f);
    public static final Color Clear       = new Color(0.0f, 0.0f, 0.0f, 0.0f);

    public static final Color Red         = new Color(1.0f, 0.0f, 0.0f);
    public static final Color Green       = new Color(0.0f, 1.0f, 0.0f);
    public static final Color Blue        = new Color(0.0f, 0.0f, 1.0f);

    // Secondaries & Tints
    public static final Color Yellow      = new Color(1.0f, 1.0f, 0.0f);
    public static final Color Cyan        = new Color(0.0f, 1.0f, 1.0f);
    public static final Color Magenta     = new Color(1.0f, 0.0f, 1.0f);
    public static final Color Orange      = new Color(1.0f, 0.5f, 0.0f);
    public static final Color Purple      = new Color(0.5f, 0.0f, 0.5f);
    public static final Color Pink        = new Color(1.0f, 0.75f, 0.8f);
    public static final Color Lime        = new Color(0.75f, 1.0f, 0.0f);
    public static final Color Teal        = new Color(0.0f, 0.5f, 0.5f);

    // Grays
    public static final Color LightGray   = new Color(0.75f);
    public static final Color Gray        = new Color(0.5f);
    public static final Color DarkGray    = new Color(0.25f);

    // UI & Game Palette Standards
    public static final Color Crimson     = new Color(0.8f, 0.1f, 0.15f);
    public static final Color Coral       = new Color(1.0f, 0.5f, 0.31f);
    public static final Color Gold        = new Color(1.0f, 0.84f, 0.0f);
    public static final Color SkyBlue     = new Color(0.53f, 0.81f, 0.98f);
    public static final Color Cornflower  = new Color(0.39f, 0.58f, 0.93f);
    public static final Color RoyalBlue   = new Color(0.25f, 0.41f, 0.88f);
    public static final Color Violet      = new Color(0.54f, 0.17f, 0.89f);
    public static final Color Emerald     = new Color(0.31f, 0.78f, 0.47f);
    public static final Color ForestGreen = new Color(0.13f, 0.55f, 0.13f);
    public static final Color Olive       = new Color(0.5f, 0.5f, 0.0f);
    public static final Color Brown       = new Color(0.6f, 0.4f, 0.2f);
}
