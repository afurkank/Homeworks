#include <iomanip>
#include "Line.h"

Line::Line(){
    this->v0 = Vec4();
    this->v1 = Vec4();
}

Line::Line(Vec4 v0, Vec4 v1){
    this->v0 = v0;
    this->v1 = v1;
}

Line::Line(Vec4 v0, Vec4 v1, Color c0, Color c1){
    this->v0 = v0;
    this->v1 = v1;
    this->c0 = c0;
    this->c1 = c1;
}