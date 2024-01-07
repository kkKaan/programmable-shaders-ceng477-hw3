#version 330 core

in vec4 color;
out vec4 fragColor;

uniform float scale;
uniform float offset;

in vec3 FragPos;

void main() {
    vec3 pos = gl_FragCoord.xyz; // Using screen coordinates

    bool x = bool(int((FragPos.x) * scale + 4.f) % 2);
    bool y = bool(int((FragPos.y) * scale) % 2);
    bool z = bool(int((FragPos.z + offset) * scale) % 2);
    bool xorXY = x != z;

    if (xorXY) {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0); // Black
    } else {
        fragColor = vec4(1.0, 1.0, 1.0, 1.0); // White
    }
}
