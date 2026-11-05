#pragma once

namespace craft::graphics {

// Saves the current back buffer as a PNG after scene measurement has finished.
[[nodiscard]] bool saveFrame(const char* path, int width, int height);

}
