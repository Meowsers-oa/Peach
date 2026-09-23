#version 410 core

in vec4 vColor;
in vec3 vNormal;
in vec2 vTexCoord;
in float vTexId;
in vec3 vFragPos;

out vec4 FragColor;

uniform sampler2D uTextures[15];

struct Light {
    int type;
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float range;
    float innerCone;
    float outerCone;
    float bias;
    bool shadow;
};
uniform Light uLights[8];
uniform int uLightCount;
uniform float uAmbientLight;
uniform sampler2DArray uShadowMaps;
uniform mat4 uSpotMatrices[8];
const float shadowNear = 0.05;

// Six array layers use the same orientation as cubemap faces.
vec3 pointShadowUV(vec3 d) {
    vec3 a = abs(d);
    vec2 uv;
    float face;
    if (a.x >= a.y && a.x >= a.z) {
        face = d.x >= 0.0 ? 0.0 : 1.0;
        uv = vec2(d.x >= 0.0 ? -d.z : d.z, -d.y) / a.x;
    } else if (a.y >= a.z) {
        face = d.y >= 0.0 ? 2.0 : 3.0;
        uv = vec2(d.x, d.y >= 0.0 ? d.z : -d.z) / a.y;
    } else {
        face = d.z >= 0.0 ? 4.0 : 5.0;
        uv = vec2(d.z >= 0.0 ? d.x : -d.x, -d.y) / a.z;
    }
    return vec3(uv * 0.5 + 0.5, face);
}

float shadowVisibility(int index, vec3 normal, vec3 toLight) {
    Light light = uLights[index];
    if (!light.shadow) return 1.0;
    // Offset the receiver towards the light in world units.
    float footprint = 2.0 * length(light.position - vFragPos)
                    / float(textureSize(uShadowMaps, 0).x);
    float bias = max(light.bias, 2.0 * footprint)
               * max(1.0, 3.0 * (1.0 - max(dot(normal, toLight), 0.0)));
    vec3 receiver = vFragPos + toLight * bias;
    vec3 coords;
    float depth;
    if (light.type == 1) {
        vec4 projected = uSpotMatrices[index] * vec4(receiver, 1.0);
        if (projected.w <= 0.0) return 1.0;
        vec3 p = projected.xyz / projected.w * 0.5 + 0.5;
        coords = vec3(p.xy, 0.0);
        depth = p.z;
    } else {
        vec3 d = receiver - light.position;
        float z = max(max(abs(d.x), abs(d.y)), abs(d.z));
        if (z <= shadowNear) return 1.0;
        coords = pointShadowUV(d);
        depth = light.range / (light.range - shadowNear)
              - light.range * shadowNear / ((light.range - shadowNear) * z);
    }
    if (depth <= 0.0 || depth >= 1.0 ||
        any(lessThan(coords.xy, vec2(0))) || any(greaterThan(coords.xy, vec2(1)))) return 1.0;
    coords.z += float(index * 6);
    vec2 texel = 1.0 / vec2(textureSize(uShadowMaps, 0).xy);
    float visibility = 0.0;
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            vec2 uv = clamp(coords.xy + vec2(x,y) * texel, texel * 0.5, 1.0 - texel * 0.5);
            float closest = texture(uShadowMaps, vec3(uv, coords.z)).r;
            visibility += depth <= closest ? 1.0 : 0.0;
        }
    }
    return visibility / 9.0;
}

void main() {
    int id = int(round(vTexId));
    vec4 texColor = vec4(1.0);

    switch (id) {
        case 0:  texColor = texture(uTextures[0],  vTexCoord); break;
        case 1:  texColor = texture(uTextures[1],  vTexCoord); break;
        case 2:  texColor = texture(uTextures[2],  vTexCoord); break;
        case 3:  texColor = texture(uTextures[3],  vTexCoord); break;
        case 4:  texColor = texture(uTextures[4],  vTexCoord); break;
        case 5:  texColor = texture(uTextures[5],  vTexCoord); break;
        case 6:  texColor = texture(uTextures[6],  vTexCoord); break;
        case 7:  texColor = texture(uTextures[7],  vTexCoord); break;
        case 8:  texColor = texture(uTextures[8],  vTexCoord); break;
        case 9:  texColor = texture(uTextures[9],  vTexCoord); break;
        case 10: texColor = texture(uTextures[10], vTexCoord); break;
        case 11: texColor = texture(uTextures[11], vTexCoord); break;
        case 12: texColor = texture(uTextures[12], vTexCoord); break;
        case 13: texColor = texture(uTextures[13], vTexCoord); break;
        case 14: texColor = texture(uTextures[14], vTexCoord); break;
        default: texColor = vec4(1.0); break;
    }

    vec4 surface = vColor * texColor;
    if (surface.a <= 0.0) discard;
    if (uLightCount == 0) {
        FragColor = surface; // Preserve the unlit renderer when no lights are registered.
        return;
    }
    vec3 normal = length(vNormal) > 0.00001 ? normalize(vNormal) : vec3(0,1,0);
    vec3 lighting = vec3(uAmbientLight);
    for (int i = 0; i < uLightCount; ++i) {
        Light light = uLights[i];
        vec3 delta = light.position - vFragPos;
        float distanceToLight = length(delta);
        if (distanceToLight >= light.range || light.intensity <= 0.0) continue;
        vec3 toLight = delta / max(distanceToLight, 0.00001);
        float cone = 1.0;
        if (light.type == 1) {
            float angle = dot(-toLight, light.direction);
            cone = smoothstep(light.outerCone, light.innerCone, angle);
        }
        float falloff = max(1.0 - pow(distanceToLight / light.range, 4.0), 0.0);
        float attenuation = falloff * falloff / (1.0 + distanceToLight * distanceToLight);
        float diffuse = max(dot(normal, toLight), 0.0);
        if (diffuse > 0.0 && cone > 0.0)
            lighting += light.color * light.intensity * attenuation * diffuse * cone
                      * shadowVisibility(i, normal, toLight);
    }
    FragColor = vec4(surface.rgb * lighting, surface.a);
}