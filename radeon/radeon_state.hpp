

/// This class gathers the program variables related to DRM and the GPU.
class radeon_state {
public:
    radeon_state(const char* device, bool exclusive);
    ~radeon_state();

    void open(const char* device, bool exclusive);
    void close();

private:
    int fd;
};
