#version 410 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec2 uResolution;
uniform float uTime;

void main() {
    FragColor = texture(uTexture, vTexCoord);
}
