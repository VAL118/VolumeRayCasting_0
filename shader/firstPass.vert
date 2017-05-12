precision mediump int;
precision mediump float;

attribute vec4 vertColor;

//
attribute vec3 position;
uniform mat4 MVP;

varying vec4 backColor;
varying vec4 pos;

//
varying vec3 worldSpaceCoords;

void main(void)
{
    backColor = vertColor;

    //// pos = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    //pos = MVP * vec4(position, 1.0);

    //gl_Position = pos;

    worldSpaceCoords = position + vec3(0.5, 0.5, 0.5); //move it from [-0.5;0.5] to [0,1]
    gl_Position = MVP * vec4( position, 1.0 );
}
