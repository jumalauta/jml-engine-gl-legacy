# GNU Rocket integration
* This directory contains GNU Rocket library: https://github.com/kusma/rocket
* The GNU Rocket library is customized
  * synceditor.c/h contains API wrappers that the engine is using. Everything else is considered "GNU Rocket internal" functionality
  * Sync editor and player (-DSYNC_PLAYER) functionality is bundled together. Demo engine will determine based on --tool command line switch and GNU Rocket server connectivity which mode should be used.
