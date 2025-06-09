# Building

To build the game you will need to install:

- SFMLv3 (https://www.sfml-dev.org)

Once the dependencies are met:

```console
cmake -B build
make -C build
./build/flappybird/run
```

Optionally, you may pass a specific resolution as a parameter:

```console
./build/flappybird/run -w 1920 -h 1080
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
