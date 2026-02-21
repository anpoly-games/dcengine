# General TODO list
## Prereq
List of prereqs which are a must before it can be published
### Structure
* cmake for the game part (static library)
* cmake for the editor (executable)
* Link dcextra2025 with this repo
* Move everything game-abstract to this repo
* Allow to specify shaders in entities
## Good to have
* ecs powered GUI or really even just a immediate mode GUI, instead of bunch of draw calls
* A way to specify resolution, fullscreen/windowed, render targets
    * In dcextra2025 we've used lowres render target, here we want configurability
