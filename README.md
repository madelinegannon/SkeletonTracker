# SkeletonTracker
An openFrameworks app that tracks and broadcasts skeletal data from a  Microsoft Kinect (V2)

- [Basic Usage and Limitations]()
- [Getting Started]()
    - [Building From Scratch]()
        - [Setting Up the Project]()
    - [Running the Exe]()


## Basic Use and Limitations
Tested with: 
- MSVS Pro 2017
- [openFrameworks v0.9.8](https://openframeworks.cc/download/older/)
- [Kinect SDK 2.0](https://www.microsoft.com/en-us/download/details.aspx?id=44561)
- [Google Protobuf 3](https://developers.google.com/protocol-buffers/)

Required openframeworks addons:
- [ofxKinectForWindows2](https://github.com/elliotwoods/ofxKinectForWindows2)
- [ofxOneEuroFilter](https://github.com/i-n-g-o/ofxOneEuroFilter)
- ofxNetwork
- ofxGui

#### Basic Controls and Usage

#### Limitations

Out of the box, this app tracks many skeletons, but will only broadcasting skeletal data of one person.

## Getting Started

### Building From Scratch
Follow these instructions when setting up on a new machine:

1. Download and Install Microsoft Visual Studios 2017.

2. Download and Install the [Kinect SDK 2.0](https://www.microsoft.com/en-us/download/details.aspx?id=44561) from Microsoft.

3. Restart your computer to finalize the MSVS and Kinect SDK setup.

    1. Verify successful installation by plugging your sensor into a USB 3.0 port and running the SDK's `Kinect Studio v2.0` app. Under the `MONITOR` tab, click the icon at the top left to toggle _Connected / Not Connected_.

5. [Download](https://openframeworks.cc/download/older/), [Install](https://openframeworks.cc/setup/vs/), and [Build](https://openframeworks.cc/learning/01_basics/how_to_add_addon_to_project/) **openFrameworks v0.9.8.**

6. Clone the required addons in your `OF_PATH/addons` folder.

    1. Each of the addons should have a README if there are more specific installation instructions.

    2. Note that for [ofxKinectForWindows2](https://github.com/elliotwoods/ofxKinectForWindows2), you need to checkout the `0.9.0` tag: 

    ```
    > git checkout 0.9.0
    ```

7. Download and Install Google Protobuf:

    Start by [installing vcpkg](https://github.com/Microsoft/vcpkg):
    
    ```
    > cd /
    > git clone https://github.com/Microsoft/vcpkg.git
    > cd vcpkg

    PS> .\bootstrap-vcpkg.bat
    PS> .\vcpkg integrate install
    ```
    
    If `vcpkg` and protobuf are already installed, you can update to the latest protobuf compilier by doing [this](https://github.com/Microsoft/vcpkg/blob/master/docs/about/faq.md#how-do-i-update-libraries).

    
    Follow Protobuf C++ Installation Instructions [for Windows](https://github.com/protocolbuffers/protobuf/blob/master/src/README.md#c-installation---windows):
    
    ```
    PS> vcpkg install protobuf protobuf:x64-windows
    ```
    
8. Build the `body.proto`:

    Navigate to `C:\vcpkg\installed\protobuf\x64-windows\tools\protobuf` and make a new directory named `body`:
        
    ```
    cd vcpkg/installed/protobuf/x64-windows/tools/protobuf
    mkdir body
    ```
       
    Copy the `\proto\body.proto` in this repo to our new `\body` directory:
        
    ```
    cp ~/path/to/repo/proto/body.proto ./body/
    ```

    Generate the C++ or Python proto files: 

    `For C++:`
    ```
    .\protoc --proto_path=body --cpp_out=body body/body.proto
    ```
        
    `For Python:`
    ```
    .\protoc --proto_path=body --python_out=body body/body.proto
     ```
 
#### Setting Up the Project
Once you have the basic components installed on your computer, follow these instructions to setup the MSVS project:

1. Open the openFrameworks Project Generator and click `import` to load the existing project directory.

   You should see the following addons selected:
       
   ```
   ofxGui
   ofxNetwork
   ofxOneEuroFilter
   ofxOsc
   ```

   **DO NOT** add ofxKinectForWindows2 yet ... we'll do that next manually.

Open the generated project in the IDE and follow the _Usage_ instructions from the [ofxKinectForWindows2 README](https://github.com/elliotwoods/ofxKinectForWindows2) to properly add the addon:

    1. Open the solution, and add the ofxKinectForWindows2Lib.vcxproj to your solution (right click on the Solution and choose _Add > Existing Project..._)

    2. In Property Manager (open it from _View -> Other Windows -> Property Manager_), right click on your project to select _Add Existing Property Sheet..._ and select the `ofxKinectForWindows2.props` file.

    3. Go back to Solution Explorer, right click on your project (e.g. 'mySketch') and select '_Add Reference..._', and add a reference to `ofxKinectForWindows2Lib`.
    
3. 