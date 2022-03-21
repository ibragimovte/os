#ifndef __MAIN_H__
#define __MAIN_H__
#define COLOR1 1
#define COLOR2 2
typedef struct Coordinates {
    int x;
    int y;
} Coordinates;

typedef enum Tower {
    UP_LEFT,       
    UP_UP,             
    UP_RIGHT,           
    MIDDLE_LEFT,        
    MIDDLE_RIGHT,        
    DOWN_LEFT,          
    DOWN_DOWN,          
    DOWN_RIGHT
} Tower;

typedef enum TankDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT
} TankDirection;

typedef struct Bullet {
    Coordinates center;
    int vector_x;
    int vector_y;
    int alive;
} Bullet;

typedef struct Tank {
    Coordinates center;    
    Tower tower;
    TankDirection dir;
    Bullet bullets[4];
    int amount_bullet;
    int PlayerId;
} Tank;


#endif
