//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  core.glsl
//
//  Core shader functionality:
//  Clipping, opacity, saturation, dimming and reflections falloff.
//

// Vertex section of code:
#if defined(VERTEX)

uniform mat4 MVPMatrix;
in vec2 positionVertex;
in vec2 texCoordVertex;
in vec4 colorVertex;

out vec2 position;
out vec2 texCoord;
out vec4 color;

void main(void)
{
    gl_Position = MVPMatrix * vec4(positionVertex.xy, 0.0, 1.0);
    position = positionVertex;
    texCoord = texCoordVertex;
    color.abgr = colorVertex.rgba;
}

// Fragment section of code:
#elif defined(FRAGMENT)

#ifdef GL_ES
precision mediump float;
#endif

in vec2 position;
in vec2 texCoord;
in vec4 color;

uniform vec4 clipRegion;
uniform float opacity;
uniform float saturation;
uniform float dimming;
uniform float reflectionsFalloff;
uniform uint shaderFlags;

uniform sampler2D textureSampler;
out vec4 FragColor;

// shaderFlags:
// 0x00000001 - Premultiplied alpha (BGRA)
// 0x00000002 - Font texture
// 0x00000004 - Post processing
// 0x00000008 - Clipping

void main()
{
    // Discard any pixels outside the clipping region.
    if (0x0u != (shaderFlags & 0x8u)) {
        if (position.x < clipRegion.x)
            discard;
        else if (position.y < clipRegion.y)
            discard;
        else if (position.x > clipRegion.z)
            discard;
        else if (position.y > clipRegion.w)
            discard;
    }

    vec4 sampledColor = texture(textureSampler, texCoord);

    // For fonts the alpha information is stored in the red channel.
    if (0x0u != (shaderFlags & 0x2u))
        sampledColor = vec4(1.0, 1.0, 1.0, sampledColor.r);

    // We need different color calculations depending on whether the texture contains
    // premultiplied alpha or straight alpha values.
    if (0x0u != (shaderFlags & 0x01u)) {
        sampledColor.rgb *= color.rgb;
        sampledColor *= color.a;
    }
    else {
        sampledColor *= color;
    }

    // When post-processing we drop the alpha channel to avoid strange issues with some
    // graphics drivers.
    if (0x0u != (shaderFlags & 0x4u))
        sampledColor.a = 1.0;

    // Opacity.
    if (opacity != 1.0) {
        if (0x0u == (shaderFlags & 0x01u))
            sampledColor.a *= opacity;
        else
            sampledColor *= opacity;
    }

    // Saturation.
    if (saturation != 1.0) {
        vec3 grayscale = vec3(dot(sampledColor.rgb, vec3(0.34, 0.55, 0.11)));
        vec3 blendedColor = mix(grayscale, sampledColor.rgb, saturation);
        sampledColor = vec4(blendedColor, sampledColor.a);
    }

    // Dimming.
    if (dimming != 1.0) {
        vec4 dimColor = vec4(dimming, dimming, dimming, 1.0);
        sampledColor *= dimColor;
    }

    // Reflections falloff.
    if (reflectionsFalloff > 0.0)
        sampledColor.argb *= mix(0.0, 1.0, reflectionsFalloff - position.y) / reflectionsFalloff;

    FragColor = sampledColor;
}
#endif
