#version 410 core

in vec2 vUV;
uniform sampler2D uScene;
uniform sampler2D uLighting;
out vec4 fragColor;

void main() {
    vec4 scene = texture(uScene, vUV);
    vec3 lighting = texture(uLighting, vUV).rgb;
    fragColor = vec4(scene.rgb * lighting, scene.a);
}
