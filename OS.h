#include "inttypes.h"
#include "vector"
#include "malloc.h"
#include "string"
#include "map"

namespace LibOS
{
    namespace SVC
    {
        //svc 0x00
        void RequestIPC(void* command)
        {
            __asm("svc 0x00");
        }

        //svc 0x01
        void Abort()
        {
            __asm("svc 0x01");

            abort();
        }
    }

    namespace IPC
    {
        enum ServiceHost 
        {
            IO,
            Graphics
        };

        struct IpcResponse
        {
            bool IsSuccessfull;
            uint64_t Responses[20];
        };

        struct IpcCommand
        {
            ServiceHost Program;
            int Command;
            void* Arguments;
            IpcResponse Response;
        };

        IpcResponse SubmitIPC(ServiceHost Host, int Commands, void* Arguments)
        {
            IpcCommand addr = {Host, Commands, Arguments};

            SVC::RequestIPC(&addr);

            return *((IpcResponse*)&addr);
        };
    }

    namespace IO
    {
        enum Commands
        {
            //Printing To Console
            _Log,

            //File 
            _OpenFileStream,
            _CloseFileStream,
            _Seek,
            _FileStreamAction,
            _GetLocation,
        };

        //TODO: Add support for ints.
        void Log(const char* addr, uint32_t size = UINT32_MAX)
        {
            struct LogArgs
            {
                const char* addr;
                uint32_t size;
            };

            LogArgs args = {addr, size};

            IPC::SubmitIPC(IPC::ServiceHost::IO,Commands::_Log, &args);
        };

        void LogLine(const char* addr)
        {
            Log(addr);
            Log("\n");
        }

        void AbortWithMesage(std::string message)
        {
            LogLine(("Error: " + message).c_str());

            SVC::Abort();
        }

        struct StreamRequest
        {
            enum Mode
            {
                Read =  1 << 0,
                Write = 1 << 1,
            };

            Mode mode;
            const char* Name;
            uint64_t Handle;
            uint64_t Destination;
            uint64_t Size;
        };

        class InFileStream
        {
        private:
            
            uint64_t _Size;

            void GetSize()
            {
                //TODO:

                SVC::Abort();
            }

        public:

            uint64_t Handle;

            InFileStream(std::string path)
            {
                StreamRequest request = {StreamRequest::Mode::Read, path.c_str()};

                Handle = IPC::SubmitIPC(IPC::ServiceHost::IO, Commands::_OpenFileStream,&request).Responses[0];

                GetSize();
            }

            ~InFileStream()
            {
                StreamRequest request = {StreamRequest::Mode::Read, nullptr, Handle};

                IPC::SubmitIPC(IPC::ServiceHost::IO, Commands::_CloseFileStream, &request);
            }

            void Read(void* Dest, int size)
            {
                StreamRequest request = {StreamRequest::Mode::Read, nullptr, Handle,(uint64_t)Dest, size};

                IPC::SubmitIPC(IPC::ServiceHost::IO, Commands::_FileStreamAction, &request);
            }

            void Seek(uint64_t Address)
            {
                StreamRequest request = {StreamRequest::Mode::Read, nullptr, Handle, Address};

                IPC::SubmitIPC(IPC::ServiceHost::IO, Commands::_Seek, &request);
            }

            void Read(uint64_t Address, void* Dest, int Size)
            {
                Seek(Address);

                Read(Dest, Size);
            }

            uint64_t Size()
            {
                return _Size;
            }
        };
    }

    namespace Graphics
    {
        enum Commands
        {
            _CreateGraphicsContext,
            _DestroyGraphicsContext,
            _RenderFrameBuffer,
        };

        uint64_t CreateGraphicsContext()
        {
            return IPC::SubmitIPC(IPC::ServiceHost::Graphics,Commands::_CreateGraphicsContext,nullptr).Responses[0];
        };

        void DestroyGraphicsContext(uint64_t Handle)
        {
            IPC::SubmitIPC(IPC::ServiceHost::Graphics, Commands::_DestroyGraphicsContext,(void*)Handle);
        }

        class FrameBuffer;
        void RenderFrameBuffer(uint64_t Handle, FrameBuffer* buffer)
        {
            IPC::SubmitIPC(IPC::ServiceHost::Graphics, Commands::_RenderFrameBuffer, buffer);
        }

        struct Color
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };

        class FrameBuffer
        {
        public:

            Color* ColorBuffer;            
            int Length;
            int Width;

            FrameBuffer(int Length, int Width)
            {
                ColorBuffer = (Color*)malloc(Length * Width * sizeof(Color));

                this->Length = Length;
                this->Width = Width;
            }

            ~FrameBuffer()
            {
                free(ColorBuffer);
            }
        };

        class GraphicsContext
        {
        public:
            std::vector<FrameBuffer*> FrameBuffers;

            uint64_t Handle;

            GraphicsContext()
            {
                Handle = CreateGraphicsContext();
            }

            ~GraphicsContext()
            {
                for (FrameBuffer* fb : FrameBuffers)
                {
                    delete fb;
                }     

                DestroyGraphicsContext(Handle);           
            }

            FrameBuffer* CreateFrameBuffer(int Width, int Height)
            {
                FrameBuffer* out = new FrameBuffer(Width, Height);

                FrameBuffers.push_back(out);

                return out;
            }

            void RenderFrameBuffer(FrameBuffer* buffer)
            {
                //TODO:
                bool IsValid = false;

                for (int i = 0; i < FrameBuffers.size(); ++i)
                {
                    if (FrameBuffers[i] == buffer)
                    {
                        IsValid = true;

                        break;
                    }
                }

                IO::AbortWithMesage("RenderFrameBuffer Currently Unsupported");
            }
        };
    }
}