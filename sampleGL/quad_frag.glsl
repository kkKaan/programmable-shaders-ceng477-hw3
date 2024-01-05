#version 330 core

in vec4 color;
out vec4 fragColor;

uniform float scale;
uniform float offset;
uniform float offsetZ;

in vec3 FragPos;

void main() {
    vec3 pos = gl_FragCoord.xyz; // Using screen coordinates

    bool x = int((FragPos.x + offset) * scale) % 2 == 0;
    bool y = int((FragPos.y + offset) * scale) % 2 == 0;
    bool z = int((FragPos.z + offsetZ) * scale) % 2 == 0;
    bool xorXY = x != y;

    if (xorXY != z) {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0); // Black
    } else {
        fragColor = vec4(1.0, 1.0, 1.0, 1.0); // White
    }
}
