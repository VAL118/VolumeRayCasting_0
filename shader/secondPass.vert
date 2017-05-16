// precision mediump int;
// precision mediump float;
//
// // attribute vec4 vertColor; // VerClr
// attribute vec3 vertColor; // VerClr
//
// //
// attribute vec3 position; // VerPos
// uniform mat4 MVP;
//
// // varying vec4 frontColor;
// // varying vec4 pos;
//
// varying vec3 EntryPoint;
// varying vec4 ExitPointCoord;
//
// void main(void)
// {
//     // frontColor = vertColor;
//     EntryPoint = vertColor;
//
//     // pos = MVP * vec4(position, 1.0);
//     ExitPointCoord = MVP * vec4(position, 1.0);
//
//     // gl_Position = pos;
//     gl_Position = ExitPointCoord;
// }



precision mediump int;
precision mediump float;

attribute vec4 vertColor;

//
attribute vec3 position; // VerPos
uniform mat4 MVP;

varying vec4 frontColor;
varying vec4 pos;

void main(void)
{
    frontColor = vertColor;

    pos = MVP * vec4(position, 1.0);
    gl_Position = pos;
}
