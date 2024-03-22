#version 330 core

out vec4 FragColor;

in vec2 texCoords;

uniform bool horizontal;
uniform sampler2D image;

const float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main() {
    vec2 texOffset = 1.f / textureSize(image, 0); // Get size of one texel

    vec3 result = texture(image, texCoords).rgb * weight[0]; // Contribution of the first texel (the one on the center)

    if(horizontal) {
        for(int x = 1; x < 5; ++x) {
            // Go both directions.
            result += texture(image, texCoords + vec2(texOffset.x * x, 0.f)).rgb * weight[x];
            result += texture(image, texCoords - vec2(texOffset.x * x, 0.f)).rgb * weight[x];
        }
    } else {
        for(int y = 1; y < 5; ++y) {
            result += texture(image, texCoords + vec2(0.f, texOffset.y * y)).rgb * weight[y];
            result += texture(image, texCoords - vec2(0.f, texOffset.y * y)).rgb * weight[y];
        }
    }

    FragColor = vec4(result, 1.f);
}
