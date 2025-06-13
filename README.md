<div align="center">
    <h2>Cobra TETR.IO Movegen</h2>

    A high-performance move generation implementation for Season 1 TETR.IO
</div>

## Overview

This project is derived from [Cobra](https://www.youtube.com/@cobra-tetris). 

Currently it runs perft from an empty position. (This may be modified as needed in src/bench.cpp)

- SRS+ rotation system
- Full-Movegen (Uses 180 spins, non-infinite SDF)
- TETR.IO Tspin detection
- Follows Season 1 ruleset for state keeping
- Optimized for single-thread performance and speed

## Building

- Requires c++20
```bash
cd src
make help # Shows build information
make -j build
```

## Links

- [YouTube Channel](https://www.youtube.com/@cobra-tetris)
- [TETR.IO Profile](https://ch.tetr.io/u/cobra)

## Credits

- **Kixenon** - Developer and maintainer
- Special thanks to **Opyu**

## License

This project is open source and available under the Apache 2.0 License.