#include <iostream>
#include <raylib.h>
#include <deque>//deque is a data structure that allows us to add and remove elements from both ends of a queue
/*In queue, we can only addadd element from last and remove element from front
In deque, we can add or remove elements from both front and back
*/
#include <raymath.h> //includes function like Vector2Add() used to add two vector2 types data

using namespace std;

static bool allowMove=false;

/* Color is a built in struct with 4 components
where, first three are the RGB components and last one is the alpha component which denotes transperancy*/
Color green = {173,204,96,255};
Color darkGreen = {43, 51,24,255};

int cellSize=30; //size of single cell or 'pixel' in grid
int cellCount=25; //no of cells in each colums and rows
//hence 30*25=750 will be the resolution or size of the window of our game
int offset = 75; //width of the border


double lastUpdateTime=0; //when last update of snake object occured, initially 0


//returns true if given element is in given deque, else returns false
bool ElementInDeque(Vector2 element, deque<Vector2> deque){
    for(unsigned int i=0; i < deque.size() ; i++)
    {
        if(Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;

}

//returns true if required interval has passed, else returns false
//if interval has passed, lastupdate time is set to current time
//in our case we put interval=0.2
bool EventTriggered(double interval){
    double currentTime= GetTime();//GetTime() is built in func which gives current time since program started
    if(currentTime - lastUpdateTime >= interval){
        lastUpdateTime=currentTime;
        return true;
    }
    return false;

}


class Snake{
    public:
    deque<Vector2> body ={Vector2{6,9}   ,  Vector2{5,9}  , Vector2{4,9}    };
    //body is a deque which stores vector2 type of data, first vector2 element is for head and remaining are for tail

    Vector2 direction={1,0}; //initial movement direction of snake 

    bool addSegment= false; //initially dont add any segment

    void Draw(){
        for( unsigned int  i=0; i<body.size(); i++){
            float x=body[i].x;
            float y=body[i].y;
            Rectangle segment=Rectangle{offset + x*cellSize,offset + y*cellSize, (float)cellSize, (float)cellSize};
            DrawRectangleRounded(segment,0.5, 6, darkGreen );
            /*
            DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color);
            where,
                struct Rectangle{
                    float x;
                    float y; //positions
                    float width;
                    float height; //sizes
                    }Recatangle;

                roundness= radius of the rounded corners
                segments= no of line segments used to draw each corner, more segmnets more smooth corner


            */
        }
    }

    void Update(){

        body.push_front(Vector2Add(body[0],direction)); // to add a new element in the front of deque body
        /* body[0] is the head of the snake and direction is added to the head, this simulates
        the head moving towards the direction vector2*/
        if(addSegment==true)
        {
            addSegment=false;
        }
        else{
            body.pop_back(); // last element of deque body is removed. now snake has only 2 elements
        }
    }

    void Reset(){
        //snake will be back to its original state after reset func is called in GameOver function
        body ={Vector2{6,9}   ,  Vector2{5,9}  , Vector2{4,9}    };
        direction = {1,0};
    }

};

class Food{
        public:
            Vector2 position;// Vector2 is a built in structure which can contain two values
                            //we use this strcut to store x and y coordinates of the food
                            //WE USE COMPUTER GRAPHICS COORDINATE SYSTEM
            Texture2D texture;

            Food(deque<Vector2> snakeBody){
                Image image=LoadImage("Graphics/food.png");
                //LoadImage is a builtin function which returns a Image
                //Image is a builtin structure containing the picture
                //Now we load this image into Texture 2D
                //Texture 2D is an optimized data type for GPU processing, faster rendering
                texture= LoadTextureFromImage(image);

                UnloadImage(image); //to free up space after loading image in texture2D

                position= GenerateRandomPos(snakeBody);

            }
            ~Food(){
                UnloadTexture(texture); //to free up space after texture after object food goes out of scope

            }
            void Draw(){
                //DrawTexture(texture_tobe_load, x-cordinate, y-cordinate, tint/color filter(optional));
                DrawTexture(texture,offset+ position.x * cellSize,offset + position.y* cellSize, WHITE);


                //DrawRectangle(x,y,w,h, color);
                //DrawRectangle(position.x *cellSize, position.y *cellSize, cellSize, cellSize, darkGreen );
                //we multiply by cellSize as the grid's smallest unit is the cellsize of 30px


            }

            Vector2 GenerateRandomCell(){
                float x = GetRandomValue(0,cellCount-1);//using float as components of vector2 are defined as float
                float y = GetRandomValue(0, cellCount-1);
                return Vector2{x,y};
            }


            Vector2 GenerateRandomPos  ( deque<Vector2> snakeBody){
                Vector2 position = GenerateRandomCell();
                while(ElementInDeque(position, snakeBody))
                {
                        position= GenerateRandomCell();

                }
                return position;
            }

};

class Game{
    public:
    Snake snake = Snake();
    Food food= Food(snake.body);//initializing the 'food' object of 'Food' class type
    bool running = true;
    int score=0; //score++ when CollisionWithFood and score=0 when GameOver

    Sound eatSound;
    Sound wallSound;

    Game(){
        InitAudioDevice();
        //Loading sounds in memory
        eatSound=LoadSound("Sounds/eat.mp3");
        wallSound=LoadSound("Sounds/wall.mp3");
    }

    ~Game(){
        //Unloading sounds to free up space
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    void Draw(){
        food.Draw();// Draw func of food object
        snake.Draw();
    }

    void Update(){
        if(running)//running becomes false only when GameOver func is executed
        {
        snake.Update();
        CheckCollisionWithFood(); //when collision with food happens, addSegment becomes true, if addsegment==true, pop_back wont happen
                                    // i.e.  an extra element is added but last element of body is not removed
                                    // hence snake grows
        CheckCollisionWithEdges();
        CheckCollisionWithTail();
       

        }
       
    }

    void CheckCollisionWithFood(){
        if(Vector2Equals(snake.body[0], food.position))
        {
           food.position = food.GenerateRandomPos(snake.body); //random pos may be within snake body, so we pass snake.body and compare it with newly generated position
            snake.addSegment=true;
            score++;
            PlaySound(eatSound);
        }
    }

    void CheckCollisionWithEdges(){
        if(snake.body[0].x == cellCount|| snake.body[0].x==-1) //cellcount for right limit, -1 for left limit
        {
            GameOver();

        }
        if(snake.body[0].y==cellCount|| snake.body[0].y==-1)
        {
            GameOver();
        }
    }

    void GameOver(){
        snake.Reset();
        food.position=food.GenerateRandomPos(snake.body); //new position for food too
        running = false;
        score=0;
        PlaySound(wallSound);
    }

    void CheckCollisionWithTail(){
        deque<Vector2> headlessBody = snake.body; //copy of snakes body
        headlessBody.pop_front(); //removes the first element from deque, ie removes head
        if(ElementInDeque(snake.body[0], headlessBody)) //if snakes head is in headlessbody deque, collision has occured
        {
            GameOver();
        }
    }

};

int main () {
        cout<<"Starting the game..."<<endl;

        //2*offset is added for extra 2*75=150px of area outside the container
        InitWindow(2*offset + cellCount*cellSize, 2*offset + cellCount*cellSize,"Retro Snake");
        /*initializing the game window with a built in function InitWindow() which
        takes 3 arguments height width and title of the game window
        */

        SetTargetFPS(60);
        //this function takes an int as arguemnt. this int is the frames rendered per seconds in our game
        //if this func is not used the computer will run the game as fast as it can
        //rate at which while loop continues

        Game game=Game();

        //here snake.update() is executed 60 times every second
            // but we need snake.update to execute snake.update() at slower rate while every other
            //code inside while loop has 60 fps
            //so we put snake.update() inside a speacial function which will execute 
            //at given new rate
            

        //WindowShouldClose() function returns true if esc key is pressed or x button on window is clicked
        while(WindowShouldClose()==false){

            BeginDrawing();//creats a blank canvas 

            if(EventTriggered(0.2)){
                    allowMove=true;
                    game.Update();
            }

            if(IsKeyPressed(KEY_UP) && game.snake.direction.y!=1 && allowMove) //making sure snake is not already moving down, so that snake doesnt do that unrealistic 180 flip
            {
                game.snake.direction={0,-1};
                game.running=true; // to restart the game if user presses any of the arrow key
                allowMove=false;
            }
            if(IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1 && allowMove)
            {
                game.snake.direction={0,1};
                game.running=true;
                allowMove=false;
            }
            if(IsKeyPressed(KEY_LEFT) && game.snake.direction.x!=1 && allowMove)
            {
                game.snake.direction={-1,0};
                game.running=true;
                allowMove=false;
            }
            if(IsKeyPressed(KEY_RIGHT) && game.snake.direction.x!=-1 && allowMove)
            {
                game.snake.direction={1,0};
                game.running=true;
                allowMove=false;
            }

   

            ClearBackground(green);//windows background is cleared with green screen
            //DrawRectangleLinesEx(rect struct, thickness, color);
            DrawRectangleLinesEx(Rectangle{
                                            (float)offset-5,
                                            (float)offset-5,
                                            (float)cellSize*cellCount+10,
                                            (float)cellSize*cellCount+10    
                                            }, 5, darkGreen );



            //DrawText(text,posX,posY,fontsize,color);
            DrawText("RETRO SNAKE",offset-5,20,40,darkGreen); //For title
            DrawText(TextFormat("%i",game.score),offset-5,offset+cellSize*cellCount+10,40,darkGreen); //For score
            //TextFormat converts any datatype to a text
            game.Draw();

            EndDrawing(); //ends the canvas drawing
        }

        CloseWindow();
        //to close the game window
        return 0;
}