/*
 * main.c
 *
 */

#include "board.h"
#include "compiler.h"
#include "intc.h"
#include "gpio.h"
#include "stdio.h"
#include "pm.h"

#include "display.h"
#include "feedbackcontroller.h"
#include "usart_interface.h"
#include "ringbuffer.h"

volatile unsigned char update_display = 0;
#define BUFFERSIZE 60
int bufferdata[BUFFERSIZE];
struct ringbuffer buffer;

#define CMD_SEND ( ('S' << 24) | ('E' << 16) | ('N' << 8) | 'D'  )
#define CMD_STOP ( ('S' << 24) | ('T' << 16) | ('O' << 8) | 'P'  )
#define CMD_SET  ( ('S' << 24) | ('E' << 16) | ('T' << 8) |  0   )

//======================================================================
//extern struct FeedbackController state;
void
main_loop( )
{
  unsigned char state = 0;
  unsigned int cmd = 0;
  unsigned char set_parameter = 0;
  unsigned char send = 0;
  unsigned int value = 0; // before .
  unsigned int fraction = 0; // after .
  unsigned int div = 1; // divisor
  signed char negative = 1;

  char string[60];
  for (;;)
  {
    if (update_display || FeedbackController_UPDATE)
    {
      sprintf( string, "r:%7.4f F:%8.5f", get_FeedbackController( FC_STATE_R ),
          1.0 / get_FeedbackController( FC_STATE_T ) );
      dip204_set_cursor_position( 1, 1 );
      dip204_write_string( string );
      sprintf( string, "I:%7.4f e:%8.5f", get_FeedbackController( FC_STATE_I ),
          get_FeedbackController( FC_STATE_E ) );
      dip204_set_cursor_position( 1, 2 );
      dip204_write_string( string );
      sprintf( string, "Y:%10.7f X:%5.2f",
          get_FeedbackController( FC_STATE_Y ), get_FeedbackController(
              FC_STATE_X ) );
      dip204_set_cursor_position( 1, 3 );
      dip204_write_string( string );
      dip204_set_cursor_position( 10, 1 );
      update_display = 0;
      FeedbackController_UPDATE = 0;
    }



    int c;
    if (ringbuffer_get( &buffer, &c ))
    {
            /*sprintf( string, "%c%c%c%c ",
                      ((cmd & 0xFF000000) >> 24),
                      ((cmd & 0xFF0000) >> 16),
                      ((cmd & 0xFF00) >> 8),
                      (cmd & 0xFF));
                  dip204_set_cursor_position (1,4);
                  dip204_write_string( string );
            static int i = 7;
            dip204_set_cursor_position( i, 4 );
            sprintf( string, "%c", (char) c );
            dip204_write_string( string );
            dip204_set_cursor_position( 10, 1 );
            i++;
            if (i > 20)
            {
              i = 7;
            }*/




      // GET COMMAND BYTE
      if (state < 4)
      {
        if ((char)c != '\0' && (char)c != '\n')
        {
          cmd |= (char) c << (8 * (3 - state));
          state++;
        }
        // PARSE CMD word
        if (state == 4)
        {
          // CMD SEND
          if (cmd == CMD_SEND)
          {
            send = 1;
            state = 0;
          }
          else if (cmd == CMD_STOP)
          {
            send = 0;
            state = 0;
          }
          else if ((cmd & 0xFFFFFF00) == CMD_SET)
          {
            cmd = cmd & 0x000000FF;
            state = 5;
            value = 0;
            fraction = 0;
            div = 1;
            negative = 1;
            if (cmd == 'F')
            {
              set_parameter = FC_STATE_T; //NOTE
            }
            else if (cmd == 'I')
            {
              set_parameter = FC_STATE_I;
            }
            else if (cmd == 'R')
            {
              set_parameter = FC_STATE_R;
            }
            else if (cmd == 'X')
            {
              set_parameter = FC_STATE_X;
            }
            else
            {
              usart_interface_write("NACK:Parameter unknown\n");
              state = 0;
            }
          }
          else
          {
            usart_interface_write("NACK:Command unknown\n");
            state = 0;
          }
          cmd = 0x00000000;
        }
      }
      // GET DIGITS
      else
      {
        // FIRST DIGIT or - ?
        if (state == 5)
        {
          if ((char)c == '-')
          {
            negative = -1;
          }
          else
          {
            state = 6;
          }
        }
        // READ DIGIT
        if (state == 6)
        {
          if ((char)c >= '0' && (char)c <= '9')
          {
            value *= 10;
            value += ((char)c) - '0';
          }
          else if ((char)c == '.')
          {
            state = 7; // NEXT digit belongs to fraction
            // SET VALUE
            double v = 0;
            if (set_parameter == FC_STATE_T)
            {
              v = (1.0 / value);
            }
            else
            {
              v = value;
              v *= negative;
            }
            set_FeedbackController(set_parameter, v);
          }
          else if ((char)c == '\n' || (char)c == '\0')
          {
            state = 0;
            // SET VALUE
            double v = 0;
            if (set_parameter == FC_STATE_T)
            {
              v = (1.0 / value);
            }
            else
            {
              v = value;
              v *= negative;
            }
            set_FeedbackController(set_parameter, v);
          }
          else
          {
            cmd = ((char)c) << 24;
            state = 1;
            // SET VALUE
            double v = 0;
            if (set_parameter == FC_STATE_T)
            {
              v = (1.0 / value);
            }
            else
            {
              v = value;
              v *= negative;
            }
            set_FeedbackController(set_parameter, v);
          }
        }
        else if (state == 7)
        {
          // FRACTION PART
          if ((char)c >= '0' && (char)c <= '9')
          {
            fraction *= 10;
            fraction += ((char)c) - '0';
            div *= 10;
          }
          else if ((char)c == '\n' || (char)c == '\0')
          {
            state = 0;
          }
          else
          {
            cmd = ((char)c) << 24;
            state = 1;
          }
          // SET VALUE
          double v = 0;
          if (set_parameter == FC_STATE_T)
          {
            v = (1.0 / (value + (fraction/(double)div)));
          }
          else
          {
            v = value + (fraction/(double)div);
            v *= negative;
          }
          set_FeedbackController(set_parameter, v);
        }
      }
    }
    // SEND or not
    if (send)
    {
      sprintf(string,"Y: %10.7f  e: %8.5f\n",
          get_FeedbackController(FC_STATE_Y),
          get_FeedbackController(FC_STATE_E));
      usart_interface_write(string);
    }
  }
}
    //
    /*
    int c;
    if (ringbuffer_get( &buffer, &c ))
    {
      static int i = 10;
      dip204_set_cursor_position( i, 4 );
      sprintf( string, "%c", (char) c );
      dip204_write_string( string );
      dip204_set_cursor_position( 10, 1 );
      i++;
      if (i > 20)
      {
        i = 10;
      }


      // state < 4 ---> get command
      if (state < 4)
      {
        cmd |= (char) c << (8 * (3 - state));
        state++;
        // state == 4 --> command word available
        if (state == 4)
        {
          if (cmd == CMD_SEND)
          {
            send = 1;
            state = 0;
          }
          else if (cmd == CMD_STOP)
          {
            send = 0;
            state = 0;
          }
          else if ((cmd & 0xFFFFFF00) == CMD_SET)
          {
            cmd = cmd & 0x000000FF;
            state = 5;
            if (cmd == 'F')
            {
              set_parameter = FC_STATE_T;
            }
            else if (cmd == 'I')
            {
              set_parameter = FC_STATE_I;
            }
            else if (cmd == 'R')
            {
              set_parameter = FC_STATE_R;
            }
            else if (cmd == 'X')
            {
              set_parameter = FC_STATE_X;
            }
            else
            {
              usart_interface_write("NACK:Parameter unknown\n");
              state = 0;
            }
          }
          else
          {
            usart_interface_write("NACK:Command unknown\n");
            state = 0;
          }
          cmd = 0;
        }
      }
      else
      {
        // check for '-'
        if (state == 5)
        {
          if ((char)c == '-')
          {
            negative = -1;
          }
          else
          {
            state = 6;
          }
        }
        // get numbers before .
        if (state == 6)
        {
          if ((char)c >= '0' && (char)c <= '9')
          {
            value *= 10;
            value += ((char)c) - '0';
          }
          else if ((char)c == '.')
          {
            state = 7;
          }
          else
          {
            state = 0;
            usart_interface_write("NACK:Not a number\n");
            // TODO
          }
        }
        else if (state == 7)
        {
          if ((char)c >= '0' && (char)c <= '9')
          {
            fraction *= 10;
            fraction += ((char)c) - '0';
            div *= 10;
          }
          else if ((char)c == '\0' || (char)c == '\n')
          {
            state = 0;
          }
          else
          {
            // GO BACK TO CMD MODE
          }
        }
      }*/


//======================================================================
/*
 * Main function. Setup of all required components, then handles controll
 * over to the main loop.
 * Returns never.
 */
int
main( void )
{
  // Switch main oscilator, frequency can be found in FOSC0
  pm_switch_to_osc0( &AVR32_PM, FOSC0, OSC0_STARTUP);

  // Setup interrupts - interrupt handling functions will be added later
  // by various component setup routines
  Disable_global_interrupt();
  INTC_init_interrupts( );
  Enable_global_interrupt();

  // Activate display dip204
  init_display( FOSC0 );
  //dip204_hide_cursor();
  ringbuffer_init( &buffer, bufferdata, BUFFERSIZE);
  init_usart_interface( FOSC0, &buffer );

  init_FeedbackController( 0, FOSC0);
  set_FeedbackController( FC_STATE_T, 1. / 200. );
  set_FeedbackController( FC_STATE_I, 1 );
  set_FeedbackController( FC_STATE_X, 0.5 );
  set_FeedbackController( FC_STATE_Y, 0 );
  set_FeedbackController( FC_STATE_U, 0 );
  set_FeedbackController( FC_STATE_R, -2 );

  start_FeedbackController( );
  update_display = 1;

  main_loop( );
  return 0;
}
