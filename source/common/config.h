#pragma once

#define IN_USE      &&(1)&&
#define NOT_IN_USE  &&(0)&&
#define USING(expr) 1 expr 1

#define DEV_BUILD IN_USE
