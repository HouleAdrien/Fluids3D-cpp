#version 150
in vec3 v_position;
in vec3 v_normal;
in float f_waterHeight;
in float f_groundHeight;
in vec4 clipSpace;
in vec2 f_texCoord;
out vec4 fragColor;

uniform vec3 light_position;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;
uniform float time;

const float microwavestrength = 0.02;
const float microwavespeed =0.03f;
in vec3 toCameraVector;

const float shineDamper = 20.0;
const float reflectivity =0.6;

void main() {

    vec3 fromLightVector = normalize(light_position -v_position) ;

    // normalized device space
    vec2 ndc = (clipSpace.xy/clipSpace.w)/2.0+0.5;
    vec2 refractTexCoords = vec2(ndc.x,ndc.y);
    vec2 reflectTexCoords = vec2(ndc.x,-ndc.y);

    vec2 distortedTexCoords = texture2D(dudvMap, vec2(f_texCoord.x + (microwavespeed*time), f_texCoord.y)).rg*0.1;
    distortedTexCoords = f_texCoord + vec2(distortedTexCoords.x, distortedTexCoords.y+(microwavespeed*time));
    vec2 totalDistortion = (texture2D(dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * microwavestrength;

    reflectTexCoords +=totalDistortion;
    reflectTexCoords.x = clamp(reflectTexCoords.x,0.001,0.999);
    reflectTexCoords.y = clamp(reflectTexCoords.y,-0.999,-0.001);

    refractTexCoords +=totalDistortion;
    refractTexCoords = clamp(refractTexCoords,0.001,0.999);

    vec4 reflectColor = texture2D(reflectionTexture,reflectTexCoords);
    vec4 refractColor = texture2D(refractionTexture,refractTexCoords);

    vec3 viewVector = normalize(toCameraVector);
    float refractivefactor = dot(viewVector,vec3(0.0,1.0,0.0));
    refractivefactor = pow(refractivefactor,0.25);
    vec3 normal =v_normal;
    normal =normalize(v_normal);

    vec3 reflectedLight = reflect(normalize(fromLightVector), normal);
    float specular = max(dot(reflectedLight, viewVector), 0.0);
    specular = pow(specular, shineDamper);
    vec3 specularHighlights = vec3(1.5, 1.5, 1.2) * specular * reflectivity;

    fragColor = mix(reflectColor,refractColor,refractivefactor);
    fragColor = mix(fragColor,vec4(0.0,0.3,0.5,2.0),0.4)+vec4(specularHighlights,0.0);

}
