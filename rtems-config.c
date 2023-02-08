
#include <bsp.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <rtems.h>
#include <rtems/bsd/bsd.h>

int main( int argc, char **argv );

static void default_set_self_prio( rtems_task_priority prio )
{
  rtems_status_code sc;

  sc = rtems_task_set_priority(RTEMS_SELF, prio, &prio);
  assert(sc == RTEMS_SUCCESSFUL);
}

static void Init( rtems_task_argument arg )
{
  const char *boot_cmdline = *((const char **) arg);
  char       *cmdline = NULL;
  int         argc = 0;
  char      **argv = NULL;
  int         result;

  /*
   * Default the syslog priority to 'debug' to aid developers.
   */
  rtems_bsd_setlogpriority("debug");

#ifdef DEFAULT_EARLY_INITIALIZATION
  early_initialization();
#endif
  
  /* Let other tasks run to complete background work */
  default_set_self_prio( RTEMS_MAXIMUM_PRIORITY - 1 );

  rtems_bsd_initialize();
  
  if ( boot_cmdline != NULL ) {
    size_t n = strlen( boot_cmdline ) + 1;

    cmdline = malloc( n );
    if ( cmdline != NULL ) {
      char* command;

      memcpy( cmdline, boot_cmdline, n);

      command = cmdline;

      /*
       * Break the line up into arguments with "" being ignored.
       */
      while ( true ) {
        command = strtok( command, " \t\r\n" );
        if ( command == NULL )
          break;

        ++argc;
        command = '\0';
      }

      /*
       * If there are arguments, allocate enough memory for the argv
       * array to be passed into main().
       *
       * NOTE: If argc is 0, then argv will be NULL.
       */
      argv = calloc( argc, sizeof( *argv ) );
      if ( argv != NULL ) {
        int a;

        command = cmdline;
        argv[ 0 ] = command;

        for ( a = 1; a < argc; ++a ) {
          command += strlen( command ) + 1;
          argv[ a ] = command;
        }
      } else {
        argc = 0;
      }
    }
  }

  result = main( argc, argv );

  free( argv );
  free( cmdline );

  exit( result );
}

/*
 * Configure LibBSD.
 */

#define RTEMS_BSD_CONFIG_NET_PF_UNIX
#define RTEMS_BSD_CONFIG_NET_IF_BRIDGE
#define RTEMS_BSD_CONFIG_NET_IF_LAGG
#define RTEMS_BSD_CONFIG_NET_IF_VLAN
#define RTEMS_BSD_CONFIG_BSP_CONFIG
#define RTEMS_BSD_CONFIG_TERMIOS_KQUEUE_AND_POLL
#define RTEMS_BSD_CONFIG_INIT
#define CONFIGURE_FILESYSTEM_NFS

#include <machine/rtems-bsd-config.h>

/*
 * Configure RTEMS.
 */

/* This is enough to get a basic main() up. */
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_STACK_CHECKER_ENABLED

/* on smaller architectures lower the number or resources */
#define CONFIGURE_UNLIMITED_OBJECTS
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS 8
#define CONFIGURE_MAXIMUM_DRIVERS 16
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 32

/* Include basic device drivers needed to call delays */
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_STUB_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_ZERO_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK

#define CONFIGURE_MAXIMUM_PROCESSORS CPU_MAXIMUM_PROCESSORS

/* #define CONFIGURE_DISABLE_BSP_SETTINGS*/

/* #define CONFIGURE_VERBOSE_SYSTEM_INITIALIZATION*/

#define CONFIGURE_INIT

#define CONFIGURE_MINIMUM_TASK_STACK_SIZE ( CPU_STACK_MINIMUM_SIZE * 4 )

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_INIT

#include <rtems/confdefs.h>

