#pragma once

/// This class wraps a file descriptor for a DRI device.
class dri_device {
public:
    /// This constructor opens a DRI device node given its pathname.
    /// This constructor calls open().
    /// \param path Pathname to the device node.
    dri_device(const char* path);
    /// The destructor closes the device.
    ~dri_device() throw();

    /// Get the file descriptor associated with this device.
    int descriptor() const { return _fd; }

protected:
    /// Open a DRI device node given its pathname.
    /// \param path Pathname to the device node.
    void open(const char* path);
    /// Close the file descriptor associated with this object.
    void close();

private:
    /// File descriptor for the DRI device.
    int _fd;
};
