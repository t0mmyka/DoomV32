#ifndef DEFINESH
#define DEFINESH

#define INTSIZE   sizeof(int)
#define FLOATSIZE sizeof(float)

#define SCREENWIDTH   640
#define SCREENCENTERX 320.0
#define SCREENHEIGHT  320
#define SCREENCENTERY 160.0

#define RIGHT         true
#define LEFT          false

#define TAU     6.283185482025146484375
#define PI3o2   4.7123889923095703125
#define PIo2    1.57079637050628662109375
#define PI3o4   2.35619449615478515625
#define PIo4    0.785398185253143310546875

#define LN2     0.693147182464599609375

#define SAVEREGS "push BP" "push R0" "push R1" "push R2" "push R3" "push R4" "push R5" "push R6" "push R7" "push R8" "push R9" "push R10" "push R11" "push R12" "push R13"
#define SETREGS  "pop R13" "pop R12" "pop R11" "pop R10" "pop R9" "pop R8" "pop R7" "pop R6" "pop R5" "pop R4" "pop R3" "pop R2" "pop R1" "pop R0" "pop BP"

void func()
{
    asm
    {
        "%define SCREENWIDTH  640"
        "%define SCREENHEIGHT 320"
        "%define INF    0x7F800000"
        "%define NEGINF 0xFF800000"

        "%define  MULTICOLOR  GPU_MultiplyColor"
        "%define  GPUTEXTURE  GPU_SelectedTexture"
        "%define  GPUREGION   GPU_SelectedRegion"
        "%define  XDRAWINGP   GPU_DrawingPointX"
        "%define  YDRAWINGP   GPU_DrawingPointY"
        "%define  GPUCOMMAND  GPU_Command"
        "%define  DRAWREGION  GPUCommand_DrawRegion"
        "%define  PADLEFT     INP_GamepadLeft"
        "%define  PADRIGHT    INP_GamepadRight"
        "%define  PADUP       INP_GamepadUp"
        "%define  PADDOWN     INP_GamepadDown"
        "%define  PADA        INP_GamepadButtonA"
        "%define  PADB        INP_GamepadButtonB"
        "%define  PADX        INP_GamepadButtonX"
        "%define  PADY        INP_GamepadButtonY"
        "%define  PADL        INP_GamepadButtonL"
        "%define  PADR        INP_GamepadButtonR"

        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   Hex print debug
        ///      Prints a raw hex value at pixel cordnitates
        ///
        ///   Usage:
        ///
        ///   Push value
        ///   Push Xpos
        ///   Push Ypos
        ///   Call _debug
        ///

        // R0    Value argument
        // R1    Print Xpos
        // R2    Print Ypos
        // R3    temp
        "_debugF:"
        "PUSH  BP"
        "MOV   BP,         SP"

        //
        // Store needed registers
        //
        "PUSH  R0"
        "PUSH  R1"
        "PUSH  R2"
        "PUSH  R3"

        //
        // Store GPU settings
        //
        "in    R0,         MULTICOLOR"
        "PUSH  R0"
        "in    R0,         GPUTEXTURE"
        "PUSH  R0"
        "in    R0,         GPUREGION"
        "PUSH  R0"
        "in    R0,         XDRAWINGP"
        "PUSH  R0"
        "in    R0,         YDRAWINGP"
        "PUSH  R0"

        //
        // Set GPU settings
        //
        "mov   R0,         0xFFFFFFFF"
        "out   MULTICOLOR, R0"
        "mov   R0,         -1"
        "out   GPUTEXTURE, R0"

        //
        // Main debug function
        //
        "mov   R0,         [BP + 4]"    //VALUE
        "mov   R1,         [BP + 3]"    //XPOS
        "mov   R2,         [BP + 2]"    //YPOS
        "out   YDRAWINGP,  R2"

        "mov   R3,         [BP + 1]"    //Old instruction pointer
        "mov   [BP + 4],   R3"          //Move old instruction pointer

        "mov   R3,         0"           //Null terminator for string
        "push  R3"

        "mov   R3,         16"          //Base #

        "push  R0"                      //Conver into string
        "push  R3"
        "call  _itoaS"

        "mov   R3,         0x78"        //Add 'x'
        "push  R3"

        "mov   R3,         0x30"        //Add '0'
        "push  R3"

        "push  R1"                      //Print the string
        "push  R2"
        "call  _printS"

        //
        // Restore GPU settings
        //
        "POP   R0"
        "out   YDRAWINGP,  R0"
        "POP   R0"
        "out   XDRAWINGP,  R0"
        "POP   R0"
        "out   GPUREGION,  R0"
        "POP   R0"
        "out   GPUTEXTURE, R0"
        "POP   R0"
        "out   MULTICOLOR, R0"

        //
        // Restore register
        //
        "POP   R3"
        "POP   R2"
        "POP   R1"
        "POP   R0"

        //
        // Return
        //
        "mov   SP,         BP"
        "POP   BP"
        "iadd  SP,         3"           //Skip the arguments

        "ret"


        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   Hex print debug
        ///      Prints a raw hex value at pixel cordnitates
        ///
        ///   Usage:
        ///
        ///   Push value
        ///   Push Xpos
        ///   Push Ypos
        ///   Call _debug
        ///

        // R0    Value argument
        // R1    Print Xpos
        // R2    Print Ypos
        // R3    Value/Char temp
        // R4    Index
        // R5    temp

        "_debug:"
        "PUSH  BP"
        "MOV   BP,         SP"

        //
        // Store all registers (not stack)
        //
        "PUSH  R0"
        "PUSH  R1"
        "PUSH  R2"
        "PUSH  R3"
        "PUSH  R4"
        "PUSH  R5"
        "PUSH  R6"
        "PUSH  R7"
        "PUSH  R8"
        "PUSH  R9"
        "PUSH  R10"
        "PUSH  R11"
        "PUSH  R12"
        "PUSH  R13"

        //
        // Store GPU settings
        //
        "in    R0,         MULTICOLOR"
        "PUSH  R0"
        "in    R0,         GPUTEXTURE"
        "PUSH  R0"
        "in    R0,         GPUREGION"
        "PUSH  R0"
        "in    R0,         XDRAWINGP"
        "PUSH  R0"
        "in    R0,         YDRAWINGP"
        "PUSH  R0"

        //
        // Set GPU settings
        //
        "mov   R0,         0xFFFFFFFF"
        "out   MULTICOLOR, R0"
        "mov   R0,         -1"
        "out   GPUTEXTURE, R0"

        //
        // Main debug function
        //
        "mov   R0,         [BP + 4]"    //VALUE
        "mov   R1,         [BP + 3]"    //XPOS
        "mov   R2,         [BP + 2]"    //YPOS
        "out   YDRAWINGP,  R2"

        "mov   R3,         [BP + 1]"    //Old instruction pointer
        "mov   [BP + 4],   R3"          //Move old instruction pointer

        "iadd  R1,         90"          //Print from the right

        "mov   R4,         0"           //Current offset

        "__debug_printLoop:"
        "mov   R3,         R0"          //Char

        "mov   R5,         R4"          //Get the bit offset
        "imul  R5,         -4"

        "shl   R3,         R5"          //Get the current char into right most pos

        "and   R3,         0x0000000F"  //Mask to a single char

        "mov   R5,         R3"          //Check if char is a letter
        "igt   R5,         9"

        "imul  R5,         7"           //digit - char offset

        "iadd  R3,         R5"          //Apply letter offset
        "iadd  R3,         0x30"        //Apply digit offset

        "out   GPUREGION,  R3"          //Draw the digit
        "out   XDRAWINGP,  R1"
        "out   GPUCOMMAND, DRAWREGION"

        "isub  R1,         10"          //Move left
        "iadd  R4,         1"           //Next char

        "mov   R5,         R4"          //Check if complete
        "ige   R5,         8"

        "jt    R5,         __debug_printLoopFinish"

        "jmp   __debug_printLoop"       //Repeat

        "__debug_printLoopFinish:"

        "out   GPUREGION,  0x78"        //Draw the 'x'
        "out   XDRAWINGP,  R1"
        "out   GPUCOMMAND, DRAWREGION"

        "isub  R1,         10"          //Move left

        "out   GPUREGION,  0x30"        //Draw the '0'
        "out   XDRAWINGP,  R1"
        "out   GPUCOMMAND, DRAWREGION"

        //
        // Restore GPU settings
        //
        "POP   R0"
        "out   YDRAWINGP,  R0"
        "POP   R0"
        "out   XDRAWINGP,  R0"
        "POP   R0"
        "out   GPUREGION,  R0"
        "POP   R0"
        "out   GPUTEXTURE, R0"
        "POP   R0"
        "out   MULTICOLOR, R0"

        //
        // Restore register
        //
        "POP   R13"
        "POP   R12"
        "POP   R11"
        "POP   R10"
        "POP   R9"
        "POP   R8"
        "POP   R7"
        "POP   R6"
        "POP   R5"
        "POP   R4"
        "POP   R3"
        "POP   R2"
        "POP   R1"
        "POP   R0"

        //
        // Return
        //
        "mov   SP,         BP"
        "POP   BP"
        "iadd  SP,         3"           //Skip the arguments

        "ret"


        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   Hex memory debug
        ///      Prints a list of mem values
        ///
        ///   Usage:
        ///
        ///   Push start value
        ///   Push end value
        ///   Call _debugmemory
        ///

        // R0    Start value
        // R1    End value
        // R2    Current value
        // R3    Xpos
        // R4    Ypos
        // R5    temp

        "_debugmemory:"
        "PUSH  BP"
        "MOV   BP,         SP"

        //
        // Store all registers (not stack)
        //
        "PUSH  R0"
        "PUSH  R1"
        "PUSH  R2"
        "PUSH  R3"
        "PUSH  R4"
        "PUSH  R5"
        "PUSH  R6"
        "PUSH  R7"
        "PUSH  R8"
        "PUSH  R9"
        "PUSH  R10"
        "PUSH  R11"
        "PUSH  R12"
        "PUSH  R13"

        //
        // Store GPU settings
        //
        "in    R0,         MULTICOLOR"
        "PUSH  R0"
        "in    R0,         GPUTEXTURE"
        "PUSH  R0"
        "in    R0,         GPUREGION"
        "PUSH  R0"
        "in    R0,         XDRAWINGP"
        "PUSH  R0"
        "in    R0,         YDRAWINGP"
        "PUSH  R0"

        //
        // Set GPU settings
        //
        "mov   R0,         0xFFFFFFFF"
        "out   MULTICOLOR, R0"
        "mov   R0,         -1"
        "out   GPUTEXTURE, R0"

        //
        // Main debug function
        //
        "mov   R0,         [BP + 3]"    //Start
        "mov   R1,         [BP + 2]"    //End

        "mov   R5,         [BP + 1]"    //Old instruction pointer
        "mov   [BP + 3],   R5"          //Move old instruction pointer

        "mov   R2,         R0"          //Loop position
        "mov   R4,         0"           //Start Y pos
        "mov   R3,         0"           //Start X pos

        "__debugmemory_loop:"
        "mov   R5,         R2"          //Test if past final address
        "igt   R5,         R1"

        "jt    R5,         __debugmemory_loopFinish"

        "out   YDRAWINGP,  R4"          //Set Y pos

        "mov   R5,         0x5B"        //Draw the '['
        "out   GPUREGION,  R5"
        "mov   R5,         R3"
        "iadd  R5,         0"
        "out   XDRAWINGP,  R5"
        "out   GPUCOMMAND, DRAWREGION"

        "mov   R5,         0x5D"        //Draw the ']'
        "out   GPUREGION,  R5"
        "mov   R5,         R3"
        "iadd  R5,         110"
        "out   XDRAWINGP,  R5"
        "out   GPUCOMMAND, DRAWREGION"

        "mov   R5,         0x3A"        //Draw the ':'
        "out   GPUREGION,  R5"
        "mov   R5,         R3"
        "iadd  R5,         120"
        "out   XDRAWINGP,  R5"
        "out   GPUCOMMAND, DRAWREGION"

        "push  R2"                      //Push the adress
        "mov   R5,         R3"          //Xpos
        "iadd  R5,         10"
        "push  R5"
        "push  R4"                      //Ypos
        "call  _debug"

        "mov   R5,         [R2]"        //Get the value
        "push  R5"                      //Push the value
        "mov   R5,         R3"          //Xpos
        "iadd  R5,         130"
        "push  R5"
        "push  R4"                      //Ypos
        "call  _debug"

        "iadd  R4,         20"          //Move down
        "mov   R5,         R4"          //Check to wrap
        "igt   R5,         340"
        "jf    R5,         __debugmemory_wrap_skip"

        "mov   R4,         0"
        "iadd  R3,         235"

        "__debugmemory_wrap_skip:"
        "iadd  R2,         1"           //Next address

        "jmp   __debugmemory_loop"

        "__debugmemory_loopFinish:"

        //
        // Restore GPU settings
        //
        "POP   R0"
        "out   YDRAWINGP,  R0"
        "POP   R0"
        "out   XDRAWINGP,  R0"
        "POP   R0"
        "out   GPUREGION,  R0"
        "POP   R0"
        "out   GPUTEXTURE, R0"
        "POP   R0"
        "out   MULTICOLOR, R0"

        //
        // Restore register
        //
        "POP   R13"
        "POP   R12"
        "POP   R11"
        "POP   R10"
        "POP   R9"
        "POP   R8"
        "POP   R7"
        "POP   R6"
        "POP   R5"
        "POP   R4"
        "POP   R3"
        "POP   R2"
        "POP   R1"
        "POP   R0"

        //
        // Return
        //
        "mov   SP,         BP"
        "POP   BP"
        "iadd  SP,         2"           //Skip the arguments

        "ret"


        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   Hex registers debug
        ///      Prints out all registers exept the stack
        ///
        ///   Usage:
        ///
        ///   Call _debugregs
        ///

        // R0    Xpos
        // R1    Ypos
        // R2    Iterator
        // R3    temp

        "_debugregs:"
        "PUSH  BP"
        "MOV   BP,         SP"

        //
        // Store all registers (not stack)
        //
        "PUSH  R0"
        "PUSH  R1"
        "PUSH  R2"
        "PUSH  R3"
        "PUSH  R4"
        "PUSH  R5"
        "PUSH  R6"
        "PUSH  R7"
        "PUSH  R8"
        "PUSH  R9"
        "PUSH  R10"
        "PUSH  R11"
        "PUSH  R12"
        "PUSH  R13"

        //
        // Add the stack pointers on for printing
        //
        "mov   R0,         [BP]"
        "push  R0"
        "mov   R0,         BP"
        "iadd  R0,         2"
        "push  R0"

        //
        // Store GPU settings
        //
        "in    R0,         MULTICOLOR"
        "PUSH  R0"
        "in    R0,         GPUTEXTURE"
        "PUSH  R0"
        "in    R0,         GPUREGION"
        "PUSH  R0"
        "in    R0,         XDRAWINGP"
        "PUSH  R0"
        "in    R0,         YDRAWINGP"
        "PUSH  R0"

        //
        // Set GPU settings
        //
        "mov   R0,         0xFFFFFFFF"
        "out   MULTICOLOR, R0"
        "mov   R0,         -1"
        "out   GPUTEXTURE, R0"

        //
        // Main debug function
        //
        "mov   R0,         320"         //Starting Xpos
        "mov   R1,         0"           //Starting Ypos
        "mov   R2,         0"           //Base pointer offset

        "__debugregs_loop:"
        "mov   R3,         R2"          //End loop if past register 15
        "igt   R3,         15"
        "jt    R3,         __debugregs_loop_end"

        "mov   R3,         0"           //Add the null terminator
        "push  R3"
        "mov   R3,         0x3A"        //Add the ':'
        "push  R3"

        "push  R2"                      //Convert the #
        "mov   R3,         10"          //Base 10
        "push  R3"
        "call  _itoaS"

        "mov   R3,         0x52"        //Add the 'R'
        "push  R3"

        "push  R0"
        "push  R1"
        "call  _printS"                 //Print the label

        "iadd  R2,         1"           //Go next index
        "mov   R3,         BP"          //Get the register value from stack
        "isub  R3,         R2"
        "mov   R3,         [R3]"

        "push  R3"
        "iadd  R0,         40"
        "push  R0"
        "push  R1"
        "call  _debug"

        "isub  R0,         40"
        "iadd  R1,         20"
        "jmp   __debugregs_loop"

        "__debugregs_loop_end:"

        //
        // Restore GPU settings
        //
        "POP   R0"
        "out   YDRAWINGP,  R0"
        "POP   R0"
        "out   XDRAWINGP,  R0"
        "POP   R0"
        "out   GPUREGION,  R0"
        "POP   R0"
        "out   GPUTEXTURE, R0"
        "POP   R0"
        "out   MULTICOLOR, R0"

        //
        // Skip stack registers that were pushed
        //
        "iadd  SP,         2"

        //
        // Restore register
        //
        "POP   R13"
        "POP   R12"
        "POP   R11"
        "POP   R10"
        "POP   R9"
        "POP   R8"
        "POP   R7"
        "POP   R6"
        "POP   R5"
        "POP   R4"
        "POP   R3"
        "POP   R2"
        "POP   R1"
        "POP   R0"

        //
        // Return
        //
        "POP   BP"

        "ret"


        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   Frame wait timer
        ///      Waits for a given amount of frames
        ///
        ///   Usage:
        ///
        ///   Push frames
        ///   Call _wait_frames
        ///
        "_wait_frames:"
        "PUSH  BP"
        "MOV   BP,         SP"

        //
        // Store registers
        //
        "PUSH  R0"

        //
        // Main frame wait function
        //
        "mov   R0,         [BP + 2]"
        "__wait_frames_loop:"
        "jf    R0,         __wait_frames_loop_end"
        "isub  R0,         1"
        "wait"
        "jmp   __wait_frames_loop"
        "__wait_frames_loop_end:"

        "mov   R0,         [BP + 1]"
        "mov   [BP + 2],   R0"

        //
        // Restore registers
        //
        "POP   R0"

        //
        // Return
        //
        "POP   BP"
        "iadd  SP,         1"
        "ret"


        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   Input wait interrupt
        ///      Waits until the player presses A/right, or B/left
        ///      A & right must be tapped, B & left may be held
        ///
        ///   Usage:
        ///
        ///   Call _input_wait
        ///
        "_input_wait:"
        "PUSH  BP"
        "MOV   BP,         SP"
        "PUSH  R0"


        //
        // Main input wait function
        //
        "__input_wait_loop:"
        "in    R0,         PADLEFT"
        "igt   R0,         0"
        "jt    R0,         __input_wait_loop_end"
        "in    R0,         PADB"
        "igt   R0,         0"
        "jt    R0,         __input_wait_loop_end"
        "in    R0,         PADRIGHT"
        "ieq   R0,         1"
        "jt    R0,         __input_wait_loop_end"
        "in    R0,         PADA"
        "ieq   R0,         1"
        "jt    R0,         __input_wait_loop_end"
        "wait"
        "jmp   __input_wait_loop"
        "__input_wait_loop_end:"
        "wait"


        //
        // Return
        //
        "POP   R0"
        "POP   BP"
        "ret"


        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   Input wait interrupt
        ///      Waits until the player presses A
        ///
        ///   Usage:
        ///
        ///   Call _input_wait
        ///
        "_wait_for_A:"
        "PUSH  BP"
        "MOV   BP,         SP"
        "PUSH  R0"


        //
        // Main input wait function
        //
        "__wait_for_A_loop:"
        "in    R0,         PADA"
        "ieq   R0,         1"
        "jt    R0,         __input_wait_loop_end"
        "wait"
        "jmp   __wait_for_A_loop"
        "__wait_for_A_loop_end:"
        "wait"


        //
        // Return
        //
        "POP   R0"
        "POP   BP"
        "ret"

        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   itoa
        ///      convert int to ascii of any base (2-16)
        ///      returns into a memory string
        ///
        ///   Usage:
        ///
        ///   Push value
        ///   Push base
        ///   Push string start
        ///   Call _itoa
        ///

        //  R0    Value
        //  R1    Base #
        //  R2    String position
        //  R3    Add value
        //  R4    table string
        //  R5    temp1
        //  R6    temp2
        "_itoa:"
        "PUSH  BP"
        "MOV   BP,         SP"

        //
        // Store all needed registers
        //
        "PUSH  R0"
        "PUSH  R1"
        "PUSH  R2"
        "PUSH  R3"
        "PUSH  R4"
        "PUSH  R5"
        "PUSH  R6"

        //
        // Main itoa function
        //
        "mov   R0,         [BP + 4]"    //VALUE
        "mov   R1,         [BP + 3]"    //BASE
        "mov   R2,         [BP + 2]"    //STRING

        "mov   R3,         [BP + 1]"    //Old instruction pointer
        "mov   [BP + 4],   R3"          //Move old instruction pointer

        "mov   R5,         R1"          //Check if base under min base
        "ilt   R5,         2"
        "jt    R5,         __itoa_return"
        "mov   R5,         R1"          //Check if base over max base
        "igt   R5,         16"
        "jt    R5,         __itoa_return"

        "mov   R6,         0"           //Push "null terminator"
        "push  R6"

        "mov   R6,         R0"          //Check if value is negative
        "ilt   R6,         0"
        "jt    R6,         __itoa_if_neg_true"
        "jf    R6,         __itoa_if_neg_false"

        "__itoa_if_neg_true:"
        "mov   R6,         R1"          //Check if base is ten
        "ieq   R6,         10"
        "jt    R6,         __itoa_if_b10_true"
        "jf    R6,         __itoa_if_b10_false"

        "__itoa_if_b10_true:"
        "mov   R5,         0x2D"
        "mov   [R2],       R5"          //Add negative sign to string
        "iadd  R2,         1"           //Go next char
        "imul  R0,         -1"          //Convert to pos
        "jmp   __itoa_if_b10_end"

        "__itoa_if_b10_false:"
        "isub  R0,         0x80000000"  //"Shift" value down
        "mov   R3,         0x40000000"  //Add value is half of "shifted"
        "jmp   __itoa_if_b10_end"

        "__itoa_if_b10_end:"
        "jmp   __itoa_if_neg_end"

        "__itoa_if_neg_false:"
        "jmp   __itoa_if_neg_end"

        "__itoa_if_neg_end:"
        "mov   R5,         0"
        "mov   R6,         0"

        //
        // Conversion loop
        //
        "__itoa_loop:"
        "mov   R6,         R0"          //Get the remainder of the value
        "imod  R6,         R1"
        "iadd  R5,         R6"          //Add the remainder to the working value
        "mov   R6,         R3"          //Get twice the remainder of the add value
        "imod  R6,         R1"
        "imul  R6,         2"
        "iadd  R5,         R6"          //Add the remainder to the working value

        "mov   R6,         R5"          //Get the remainder of the working value
        "imod  R6,         R1"

        "mov   R4,         __itoa_convert_table"
        "iadd  R4,         R6"
        "mov   R6,         [R4]"
        "push  R6"

        "idiv  R5,         R1"          //Carry the extra working value

        "idiv  R0,         R1"          //Divide value by base #
        "idiv  R3,         R1"          //Divide add value by base #

        "jt    R0,         __itoa_loop" //Repeat if any values are non zero
        "jt    R3,         __itoa_loop"
        "jt    R5,         __itoa_loop"


        //
        // Moves string from stack to memory
        //
        "__itoa_string_loop:"
        "pop   R6"                      //Get the current char
        "mov   R5,         R6"          //End if null
        "ieq   R5,         0"
        "jt    R5,         __itoa_string_loop_end"

        "mov   [R2],       R6"          //Add char
        "iadd  R2,         1"           //Go next char
        "jmp   __itoa_string_loop"

        "__itoa_string_loop_end:"

        "mov   R5,         0"
        "mov   [R2],       R5"          //Add null terminator

        "__itoa_return:"

        //
        // Restore register
        //
        "POP   R6"
        "POP   R5"
        "POP   R4"
        "POP   R3"
        "POP   R2"
        "POP   R1"
        "POP   R0"

        //
        // Return
        //
        "mov   SP,         BP"
        "POP   BP"
        "iadd  SP,         3"           //Skip the arguments

        "ret"

        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   itoa stack
        ///      convert int to ascii of any base (2-16)
        ///      returns into the stack
        ///      *NOTE: does not add the null terminator
        ///
        ///   Usage:
        ///
        ///   *Push null terminator (optional)
        ///   Push value
        ///   Push base
        ///   Call _itoaS
        ///

        //  R0    Value
        //  R1    Base #
        //  R2    negative flag
        //  R3    Add value
        //  R4    table string
        //  R5    temp1
        //  R6    temp2
        //  R7    stack temp
        "_itoaS:"
        "isub  SP,         31"          //Leave room for string + and return data

        //
        // Store all needed registers
        //
        "PUSH  BP"
        "PUSH  R0"
        "PUSH  R1"
        "PUSH  R2"
        "PUSH  R3"
        "PUSH  R4"
        "PUSH  R5"
        "PUSH  R6"
        "PUSH  R7"

        //
        // Move to the start of the stack
        //
        "MOV   BP,         SP"
        "iadd  BP,         43"
        "mov   SP,         BP"


        //
        // Main itoa string function
        //
        "mov   R0,         [BP - 1]"    //VALUE
        "mov   R1,         [BP - 2]"    //BASE
        "mov   R7,         [BP - 3]"    //Instruction pointer

        "mov   R5,         R1"          //Check if base under min base
        "ilt   R5,         2"
        "jt    R5,         __itoaS_return"
        "mov   R5,         R1"          //Check if base over max base
        "igt   R5,         16"
        "jt    R5,         __itoaS_return"

        "mov   R3,         0"           //Add value is set to zero

        "mov   R6,         R0"          //Check if value is negative
        "ilt   R6,         0"
        "jt    R6,         __itoaS_if_neg_true"
        "jf    R6,         __itoaS_if_neg_false"

        "__itoaS_if_neg_true:"
        "mov   R6,         R1"          //Check if base is ten
        "ieq   R6,         10"
        "jt    R6,         __itoaS_if_b10_true"
        "jf    R6,         __itoaS_if_b10_false"

        "__itoaS_if_b10_true:"
        "mov   R2,         1"           //Set negative flag true
        "imul  R0,         -1"
        "jmp   __itoaS_if_b10_end"

        "__itoaS_if_b10_false:"
        "mov   R2,         0"           //Set negative flag false
        "isub  R0,         0x80000000"  //"Shift" value down
        "mov   R3,         0x40000000"  //Add value is half of "shifted"
        "jmp   __itoaS_if_b10_end"

        "__itoaS_if_b10_end:"
        "jmp   __itoaS_if_neg_end"

        "__itoaS_if_neg_false:"
        "mov   R2,         0"           //Set negative flag false
        "jmp   __itoaS_if_neg_end"

        "__itoaS_if_neg_end:"

        "mov   R5,         0"
        "mov   R6,         0"

        //
        // Conversion loop
        //
        "__itoaS_loop:"
        "mov   R6,         R0"          //Get the remainder of the value
        "imod  R6,         R1"
        "iadd  R5,         R6"          //Add the remainder to the working value
        "mov   R6,         R3"          //Get twice the remainder of the add value
        "imod  R6,         R1"
        "imul  R6,         2"
        "iadd  R5,         R6"          //Add the remainder to the working value

        "mov   R6,         R5"          //Get the remainder of the working value
        "imod  R6,         R1"

        "mov   R4,         __itoa_convert_table"
        "iadd  R4,         R6"
        "mov   R6,         [R4]"
        "push  R6"

        "idiv  R5,         R1"          //Carry the extra working value

        "idiv  R0,         R1"          //Divide value by base #
        "idiv  R3,         R1"          //Divide add value by base #

        "jt    R0,         __itoaS_loop" //Repeat if any values are non zero
        "jt    R3,         __itoaS_loop"
        "jt    R5,         __itoaS_loop"

        "__itoaS_loop_end:"


        "jt    R2,         __itoaS_if_negFlag_true"
        "jf    R2,         __itoaS_if_negFlag_false"

        "__itoaS_if_negFlag_true:"
        "mov   R2,         0x2D"        //Add negative sign
        "push  R2"
        "jmp __itoaS_if_negFlag_end"

        "__itoaS_if_negFlag_false:"
        "jmp __itoaS_if_negFlag_end"

        "__itoaS_if_negFlag_end:"


        "__itoaS_return:"

        "push  R7"                      //Add return value after string
        "mov   [BP - 34],  SP"          //Add adress to return from to data clump
        "mov   SP,         BP"          //Go to data clump
        "isub  SP,         43"

        //
        // Restore register
        //
        "POP   R7"
        "POP   R6"
        "POP   R5"
        "POP   R4"
        "POP   R3"
        "POP   R2"
        "POP   R1"
        "POP   R0"
        "POP   BP"

        //
        // Return
        //
        "POP   SP"
        "isub  SP,         1"           //Offset because POP changes SP after popping
        "ret"



        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   base 2-16 ascii conversion table
        ///
        "__itoa_convert_table:"
        "string \"0123456789ABCDE\""


        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   print stack right justified
        ///      prints ascii text from the stack
        ///      warning: loops until it hits a null terminator
        ///
        ///   Usage:
        ///
        ///   *stack=string
        ///   Push Xpos
        ///   Push Ypos
        ///   Call _printSr
        ///

        //  R0    Xpos
        //  R1    Ypos
        //  R2    char
        //  R3    test

        "_printSr:"

        "PUSH  BP"
        "MOV   BP,         SP"

        //
        // Store all needed registers
        //
        "PUSH  R0"
        "PUSH  R1"
        "PUSH  R2"
        "PUSH  R3"

        //
        // Store GPU settings
        //
        "in    R0,         GPUTEXTURE"
        "PUSH  R0"
        "in    R0,         GPUREGION"
        "PUSH  R0"
        "in    R0,         XDRAWINGP"
        "PUSH  R0"
        "in    R0,         YDRAWINGP"
        "PUSH  R0"

        //
        // Main print function
        //
        "mov   R0,         [BP + 3]" //XPOS
        "mov   R1,         [BP + 2]" //YPOS

        "mov   SP,         BP"       //Start from before the call
        "iadd  SP,         4"

        "__printSr_null_search:"
        "mov   R3,         [SP]"
        "ieq   R3,         0"
        "jt    R3,         __printSr_null_found"
        "iadd  SP,         1"
        "jmp   __printSr_null_search"

        "__printSr_null_found:"
        "mov   R3,         SP"       //Determine X offset
        "isub  R3,         BP"
        "isub  R3,         5"
        "imul  R3,         10"

        "isub  R0,         R3"       //Shift X pos

        "out   GPUTEXTURE, -1"       //Set texture
        "out   YDRAWINGP,  R1"       //Set Y pos

        "mov   SP,         BP"       //Start from before the call
        "iadd  SP,         4"

        "__printSr_loop:"
        "pop   R2"
        "mov   R3,         R2"       //Check if null terminator
        "ieq   R3,         0"
        "jt    R3,         __printSr_loop_end"

        "out   GPUREGION,  R2"       //Draw the char
        "out   XDRAWINGP,  R0"
        "out   GPUCOMMAND, DRAWREGION"

        "iadd  R0,         10"       //Shift X and loop
        "jmp   __printSr_loop"

        "__printSr_loop_end:"

        "mov   R0,         [BP + 1]" //Move return pointer
        "push  R0"

        "mov   R0,         [BP]"     //Move old base pointer
        "push  R0"

        "mov   R0,         [BP - 1]" //Move old registers
        "push  R0"
        "mov   R0,         [BP - 2]" //Move old registers
        "push  R0"
        "mov   R0,         [BP - 3]" //Move old registers
        "push  R0"
        "mov   R0,         [BP - 4]" //Move old registers
        "push  R0"
        "mov   R0,         [BP - 5]" //Move old settings
        "push  R0"
        "mov   R0,         [BP - 6]" //Move old settings
        "push  R0"
        "mov   R0,         [BP - 7]" //Move old settings
        "push  R0"
        "mov   R0,         [BP - 8]" //Move old settings
        "push  R0"

        //
        // Restore GPU settings
        //
        "POP   R0"
        "out   YDRAWINGP,  R0"
        "POP   R0"
        "out   XDRAWINGP,  R0"
        "POP   R0"
        "out   GPUREGION,  R0"
        "POP   R0"
        "out   GPUTEXTURE, R0"

        //
        // Restore register
        //
        "POP   R3"
        "POP   R2"
        "POP   R1"
        "POP   R0"

        //
        // Return
        //
        "POP   BP"

        "ret"


        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   print stack
        ///      prints ascii text from the stack
        ///      warning: loops until it hits a null terminator
        ///
        ///   Usage:
        ///
        ///   *stack=string
        ///   Push Xpos
        ///   Push Ypos
        ///   Call _printS
        ///

        //  R0    Xpos
        //  R1    Ypos
        //  R2    char
        //  R3    test

        "_printS:"

        "PUSH  BP"
        "MOV   BP,         SP"

        //
        // Store all needed registers
        //
        "PUSH  R0"
        "PUSH  R1"
        "PUSH  R2"
        "PUSH  R3"

        //
        // Store GPU settings
        //
        "in    R0,         GPUTEXTURE"
        "PUSH  R0"
        "in    R0,         GPUREGION"
        "PUSH  R0"
        "in    R0,         XDRAWINGP"
        "PUSH  R0"
        "in    R0,         YDRAWINGP"
        "PUSH  R0"

        //
        // Main print function
        //
        "mov   R0,         [BP + 3]" //XPOS
        "mov   R1,         [BP + 2]" //YPOS

        "out   GPUTEXTURE, -1"       //Set texture
        "out   YDRAWINGP,  R1"       //Set Y pos

        "mov   SP,         BP"       //Start from before the call
        "iadd  SP,         4"

        "__printS_loop:"
        "pop   R2"
        "mov   R3,         R2"       //Check if null terminator
        "ieq   R3,         0"
        "jt    R3,         __printS_loop_end"

        "out   GPUREGION,  R2"       //Draw the char
        "out   XDRAWINGP,  R0"
        "out   GPUCOMMAND, DRAWREGION"

        "iadd  R0,         10"       //Shift X and loop
        "jmp   __printS_loop"

        "__printS_loop_end:"

        "mov   R0,         [BP + 1]" //Move return pointer
        "push  R0"

        "mov   R0,         [BP]"     //Move old base pointer
        "push  R0"

        "mov   R0,         [BP - 1]" //Move old registers
        "push  R0"
        "mov   R0,         [BP - 2]" //Move old registers
        "push  R0"
        "mov   R0,         [BP - 3]" //Move old registers
        "push  R0"
        "mov   R0,         [BP - 4]" //Move old registers
        "push  R0"
        "mov   R0,         [BP - 5]" //Move old settings
        "push  R0"
        "mov   R0,         [BP - 6]" //Move old settings
        "push  R0"
        "mov   R0,         [BP - 7]" //Move old settings
        "push  R0"
        "mov   R0,         [BP - 8]" //Move old settings
        "push  R0"

        //
        // Restore GPU settings
        //
        "POP   R0"
        "out   YDRAWINGP,  R0"
        "POP   R0"
        "out   XDRAWINGP,  R0"
        "POP   R0"
        "out   GPUREGION,  R0"
        "POP   R0"
        "out   GPUTEXTURE, R0"

        //
        // Restore register
        //
        "POP   R3"
        "POP   R2"
        "POP   R1"
        "POP   R0"

        //
        // Return
        //
        "POP   BP"

        "ret"


        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   print memory
        ///      prints ascii text from memory
        ///      warning: loops until it hits a null terminator
        ///
        ///   Usage:
        ///
        ///   Push string address
        ///   Push Xpos
        ///   Push Ypos
        ///   Call _printMEM
        ///

        //  R0    Address
        //  R1    Xpos
        //  R2    Ypos
        //  R3    char
        //  R4    test

        "_printMEM:"
        "PUSH  BP"
        "MOV   BP,         SP"

        //
        // Store all needed registers
        //
        "PUSH  R0"
        "PUSH  R1"
        "PUSH  R2"
        "PUSH  R3"
        "PUSH  R4"

        //
        // Store GPU settings
        //
        "in    R0,         GPUTEXTURE"
        "PUSH  R0"
        "in    R0,         GPUREGION"
        "PUSH  R0"
        "in    R0,         XDRAWINGP"
        "PUSH  R0"
        "in    R0,         YDRAWINGP"
        "PUSH  R0"

        //
        // Initialize data
        //
        "mov   R0,         [BP + 4]" //ADDRESS
        "mov   R1,         [BP + 3]" //XPOS
        "mov   R2,         [BP + 2]" //YPOS

        "out   GPUTEXTURE, 5"        //Set texture
        "out   YDRAWINGP,  R2"       //Set Y pos
        "jmp   __printMEM_loop"

        //
        // Print loop
        //
        "__printMEM_newline:"
        "mov   R1,         [BP + 3]" //Move X pos
        "iadd  R2,         20"       //Move Y pos
        "out   YDRAWINGP,  R2"       //Set Y pos
        "iadd  R0,         1"        //Next Char

        "__printMEM_loop:"
        "mov   R3,         [R0]"     //Get Char

        "mov   R4,         R3"       //Text for newline
        "ieq   R4,         0x0A"
        "jt    R4,         __printMEM_newline"
        "mov   R4,         R3"
        "ieq   R4,         0x0D"
        "jt    R4,         __printMEM_newline"

        "mov   R4,         R3"       //Test for null terminator
        "ieq   R4,         0x00"
        "jt    R4,         __printMEM_loop_end"

        "out   GPUREGION,  R3"       //Draw the char
        "out   XDRAWINGP,  R1"
        "out   GPUCOMMAND, DRAWREGION"

        "iadd  R1,         10"       //Move X pos
        "iadd  R0,         1"        //Next Char
        "jmp   __printMEM_loop"

        "__printMEM_loop_end:"
        "mov   R0,         [BP + 1]" //Get instruction pointer
        "mov   [BP + 4],   R0"       //Move instruction pointer

        //
        // Restore GPU settings
        //
        "POP   R0"
        "out   YDRAWINGP,  R0"
        "POP   R0"
        "out   XDRAWINGP,  R0"
        "POP   R0"
        "out   GPUREGION,  R0"
        "POP   R0"
        "out   GPUTEXTURE, R0"

        //
        // Restore register
        //
        "POP   R4"
        "POP   R3"
        "POP   R2"
        "POP   R1"
        "POP   R0"

        //
        // Return
        //
        "POP   BP"
        "iadd  SP,         3"
        "ret"


        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        ///
        ///   memory string length
        ///      gets the length of a string from
        ///      warning: loops until it hits a null terminator
        ///
        ///   Usage:
        ///
        ///   Push string address
        ///   Call _stringLenMEM
        ///   Pop  length
        ///

        //  R0    Address
        //  R1    Counter
        //  R2    Test

        "_stringLenMEM:"
        "PUSH  BP"
        "MOV   BP,         SP"

        //
        // Store all needed registers
        //
        "PUSH  R0"
        "PUSH  R1"
        "PUSH  R2"

        //
        // Initialize data
        //
        "mov   R0,         [BP + 2]"
        "mov   R1,         -1"       //Start at -1 to exlude terminator

        "__stringLenMEM_loop:"
        "iadd  R1,         1"
        "mov   R2,         [R0]"
        "iadd  R0,         1"
        "jt    R2,         __stringLenMEM_loop"

        "mov   [BP + 2],   R1"

        //
        // Restore register
        //
        "POP   R2"
        "POP   R1"
        "POP   R0"

        //
        // Return
        //
        "POP   BP"

        "ret"
    }
}

#endif
