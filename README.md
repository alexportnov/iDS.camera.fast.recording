# iDS.camera.fast.recording
Simple utility to record video from iDS cameras at high frame rates.
Depends on their camera you are using, but the code itself is fully capable running on 800fps.

This is as simple as it gets - single file - everything hardcoded.

## Install
Get the [iDS drivers and SDK](https://en.ids-imaging.com/download-ueye-lin64.html).

You should have [OpenCV](https://github.com/opencv/opencv) installed to compile.

```sh
make
./framecap ./out.avi 5
```

Thats about it; it will record 5 sec video.

Have fun!
