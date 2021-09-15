#version 440

layout(std430, binding = 0) buffer pixelBuffer {
	vec3 pixels[];
};

out vec4 outputColor;
uniform int bytesPerPixel;
uniform int imageWidth;
uniform int imageHeight;



void main()
{
    int index = int(gl_FragCoord.x) + int(gl_FragCoord.y * imageWidth);
	//float r = pixels[index] / 255.0;
	//float g = pixels[index + 1] / 255.0;
	//float b = pixels[index + 2] / 255.0;
	//vec4 pixel = imageLoad(sortedImage, ivec2(gl_FragCoord.x, imageHeight - gl_FragCoord.y));
	vec3 pixel = pixels[index];

	outputColor = vec4(pixel.r, pixel.g, pixel.b, 1.0);
}