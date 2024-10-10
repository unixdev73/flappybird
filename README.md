# Building

To build the game you will need to install:

- SFML
- Box2D

On debian-based distros (ubuntu / wsl2 / mint),
you should be able to get the pre-requisites like so:

```console
sudo apt install libbox2d-dev libsfml-dev
```

Once the dependencies are met:

```console
cmake -B build
make -C build
./build/flappybird/run
```

Optionally, you may pass a specific resolution as a parameter:

```console
./build/flappybird/run 1920 1080
```

# How to play

The aim of the game is to keep flying as long as possible.
With each obstacle passed, the score is increased.

Press space to flap the bird's wings, when in game.
That's all there is to it.
Enjoy!

# Contributors

The sprite sheet for the bird was created by
ma9ici4n (https://itch.io/profile/ma9ici4n).
