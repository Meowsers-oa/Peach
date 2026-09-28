#version 410 core

uniform mat4 uViewProjection;
uniform vec3 uLight;

out vec2 vOffset;

void main() {
    vec2 corners[6] = vec2[6](
        vec2(-1, -1), vec2(1, -1), vec2(1, 1),
        vec2(1, 1), vec2(-1, 1), vec2(-1, -1)
    );
    vOffset = corners[gl_VertexID];
    gl_Position = uViewProjection * vec4(uLight.xy + vOffset * uLight.z, 0, 1);
}
