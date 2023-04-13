//
// Created by 98max on 12/04/2023.
//

#ifndef MAXOS_COMMON_INPUTSTREAM_H
#define MAXOS_COMMON_INPUTSTREAM_H

#include <common/vector.h>

namespace maxOS{

    namespace common{

        // Forward declaration
        template<class Type> class GenericInputStream;

        template<class Type> class InputStreamEventHandler
        {
            friend class GenericInputStream<Type>;

            protected:
                common::Vector<GenericInputStream<Type>*> genericInputStreams;
            public:
                InputStreamEventHandler();
                ~InputStreamEventHandler();

                virtual void onRead(Type readElement);
                virtual void onEndOfStream(GenericInputStream<Type>* stream);
        };

        template<class Type, class ProcessorType> class InputStreamProcessor : public InputStreamEventHandler<Type>, public GenericInputStream<ProcessorType>
        {
            public:
                InputStreamProcessor();
                InputStreamProcessor(InputStreamEventHandler<ProcessorType>* genericStreamEventHandler);
                ~InputStreamProcessor();

                void onEndOfStream(GenericInputStream<Type>* stream);
        };


        template<class Type> class InputStreamSocket : public InputStreamProcessor<Type, Type>
        {
            public:
                InputStreamSocket();
                InputStreamSocket(InputStreamEventHandler<Type>* processorHandler);
                ~InputStreamSocket();

                void onRead(Type readElement);
        };



        template<class Type> class InputStreamBuffer : protected InputStreamProcessor<Type, Type*>
        {
            protected:
                Type buffer[10240]; // Buffer o
                int offset;
                Type eventFireElement;
                Type terminationElement;

            public:
                InputStreamBuffer(Type eventFireElement, Type terminationElement));
                ~InputStreamBuffer();

                void onRead(Type readElement);
                void onEndOfStream(GenericInputStream<Type>* stream);
                void flush();
        };

        template<class Type> class GenericInputStream{

            protected:
                common::Vector<InputStreamEventHandler<Type>*> inputStreamEventHandlers;

            public:
                GenericInputStream();
                GenericInputStream(InputStreamEventHandler<Type>* inputStreamEventHandler);
                ~GenericInputStream();

                void connectEventHandler(InputStreamEventHandler<Type>* inputStreamEventHandler);
                void diconnectEventHandler(InputStreamEventHandler<Type>* inputStreamEventHandler);

        };

        // The ">>" operator is used to read data from a stream, it takes a stream to read from and an event handler to handle the data
        template<class Type> void operator>>(GenericInputStream<Type>& source, InputStreamEventHandler<Type>& inputStreamEventHandler);

        // The ">>" operator is used to read data from a stream, it takes a stream to read from and a processor to convert the data into another type
        template<class Type, class ProcessorType> GenericInputStream<ProcessorType>& operator>>(GenericInputStream<Type>& source, InputStreamProcessor<Type, ProcessorType>& processor);

        class InputStream : public GenericInputStream<string>
        {
            public:
                InputStream(InputStreamEventHandler<string>* inputStreamEventHandler);
        };


        ///_______________________________________________TEMPLATES_________________________________________________________________///

        /**
         * @details Creates a new InputStreamProcessor
         * @tparam Type the type of data the stream is handling
         */
        template<class Type> InputStreamEventHandler<Type>::InputStreamEventHandler() {

        }

        /**
         * @details Destroys the InputStreamProcessor and disconnects it from all streams
         * @tparam Type the type of data the stream is handling
         */
        template<class Type> InputStreamEventHandler<Type>::~InputStreamEventHandler() {
            // Loop through all the streams and disconnect this handler from them
            while(!genericInputStreams.empty())
                (*genericInputStreams.begin()) -> diconnectEventHandler(this);

        }

        /**
         * @details Called when data is read from a stream (overridden by subclasses)
         * @tparam Type the type of data the stream is handling
         * @param readElement The element that was read from the stream
         */
        template<class Type> void InputStreamEventHandler<Type>::onRead(Type readElement) {

        }

        /**
         * @details Called when a stream has finished. Removes the stream from the array of streams
         * @tparam Type The type of data the stream is handling
         * @param stream The stream that has finished
         */
        template<class Type> void InputStreamEventHandler<Type>::onEndOfStream(GenericInputStream<Type> *stream) {
            // Remove the stream from the array of streams as it has finished
            genericInputStreams.erase(stream);
        }


        /**
         * @details Creates a new InputStreamProcessor
         * @tparam Type The type of data the stream is handling
         * @tparam ProcessorType The type of data to convert the data into via the processor
         */
        template<class Type, class ProcessorType> InputStreamProcessor<Type, ProcessorType>::InputStreamProcessor()
        : InputStreamEventHandler<Type>(),
          GenericInputStream<ProcessorType>()
        {

        }

        /**
         * @details Creates a new InputStreamProcessor
         * @tparam Type The type of data the stream is handling
         * @tparam ProcessorType The type of data to convert the data into via the processor
         * @param genericStreamEventHandler The event handler to handle the data (passed to the GenericInputStream constructor)
         */
        template<class Type, class ProcessorType> InputStreamProcessor<Type, ProcessorType>::InputStreamProcessor(InputStreamEventHandler<ProcessorType> *genericStreamEventHandler)
        : InputStreamEventHandler<Type>(),
          GenericInputStream<ProcessorType>(genericStreamEventHandler)
        {

        }

        /**
         * @details Destroys the InputStreamProcessor
         * @tparam Type  The type of data the stream is handling
         * @tparam ProcessorType The type of data to convert the data into via the processor
         */
        template<class Type, class ProcessorType> InputStreamProcessor<Type, ProcessorType>::~InputStreamProcessor() {

        }

        /**
         * @details Called when a stream has finished. Passes the event on to the internetProtocolHandlers and then removes the stream from the array of streams
         * @tparam Type The type of data the stream is handling
         * @tparam ProcessorType The type of data to convert the data into via the processor
         * @param stream The stream that has finished
         */
        template<class Type, class ProcessorType> void InputStreamProcessor<Type, ProcessorType>::onEndOfStream(GenericInputStream<Type>* stream) {

            // Loop through and run the end of stream event on the handlers
            for(typename Vector<InputStreamEventHandler<ProcessorType>*>::iterator inputStreamEventHandler = this -> inputStreamEventHandlers.begin(); inputStreamEventHandler != this -> inputStreamEventHandlers.end(); inputStreamEventHandler++)
                (*inputStreamEventHandler) -> onEndOfStream(this);

            // Run the default end of stream event, this removes the stream from the array of streams
            InputStreamEventHandler<Type>::onEndOfStream(stream);

        }

        /**
         * @details Creates a new InputStreamSocket
         * @tparam Type The type of data the stream is handling
         */
        template<class Type> InputStreamSocket<Type>::InputStreamSocket() {

        }

        /**
         * @details Creates a new InputStreamSocket and connects it to the handler
         * @tparam Type The type of data the stream is handling (and the sockets processor type)
         * @param processorHandler The handler to pass to InputStreamProcessor
         */
        template<class Type> InputStreamSocket<Type>::InputStreamSocket(InputStreamEventHandler<Type> *processorHandler)
        : InputStreamProcessor<Type, Type>(processorHandler)
        {

        }

        /**
         * @details Destroys the InputStreamSocket
         * @tparam Type The type of data the stream is handling
         */
        template<class Type> InputStreamSocket<Type>::~InputStreamSocket() {

        }

        /**
         * @details Called when data is read from a stream. Passes the event on to the internetProtocolHandlers
         * @tparam Type The type of data the stream is handling
         * @param readElement The element that was read from the stream
         */
        template<class Type> void InputStreamSocket<Type>::onRead(Type readElement) {

            // Pass the read event on to the handlers
            for(typename Vector<InputStreamEventHandler<Type>*>::iterator inputStreamEventHandler = this -> inputStreamEventHandlers.begin(); inputStreamEventHandler != this -> inputStreamEventHandlers.end(); inputStreamEventHandler++)
                (*inputStreamEventHandler) -> onRead(readElement);

        }

        /**
         * @details Creates a new InputStreamBuffer
         * @tparam Type The type of data the stream is handling
         * @param eventFireElement The element to fire the event on
         * @param terminationElement The element that signifies the end of the part of the stream to be buffered (e.g a newline)
         */
        template<class Type> InputStreamBuffer<Type>::InputStreamBuffer(Type eventFireElement, Type terminationElement) {
            // Nothing has been read so the buffer offset is at the start
            offset = 0;

            // Set the event fire and termination elements
            this -> eventFireElement = eventFireElement;
            this -> terminationElement = terminationElement;
        }

        /**
         * @details Destroys the InputStreamBuffer
         * @tparam Type The type of data the stream is handling
         */
        template<class Type> InputStreamBuffer<Type>::~InputStreamBuffer() {

        }

        /**
         * @details Called when data is read from a stream. Adds the data to the buffer and checks if the event should be fired
         * @tparam Type
         * @param readElement
         */
        template<class Type> void InputStreamBuffer<Type>::onRead(Type readElement) {

            // If this element should fire an event
            if(readElement == eventFireElement){
                // Fire the event by flushing the buffer
                flush();
                return;
            }

            // If the offset is past the length of the buffer then flush the buffer
            if(offset >= 10238){
                flush();
            }

            // Add the element to the buffer
            buffer[offset++] = readElement;

        }

        /**
         * @details Called when a stream has finished. Flushes the buffer if there is any data in it
         * @tparam Type The type of data the stream is handling
         * @param stream The stream that has finished
         */
        template<class Type> void InputStreamBuffer<Type>::onEndOfStream(GenericInputStream<Type> *stream) {

            // If data had been read before the stream ends then flush the buffer
            if(offset > 0)
                flush();

            // Run the default end of stream event
            InputStreamProcessor<Type, Type*>::onEndOfStream(stream);

        }

        /**
         * @details Flushes the buffer by adding the termination element and firing an on read event (NOTE: The buffer is not cleared after this just overwritten)
         * @tparam Type The type of data the stream is handling
         */
        template<class Type> void InputStreamBuffer<Type>::flush() {

            // If there is no data in the buffer then return
            if(offset == 0)
                return;

            // Add the termination element to the buffer
            buffer[offset] = terminationElement;

            // Fire the on read event
            InputStreamProcessor<Type, Type*>::onRead(buffer);

            // Reset the offset
            offset = 0;

        }

        /**
         * @details Creates a new GenericInputStream
         * @tparam Type The type of data the stream is handling
         */
        template<class Type> GenericInputStream<Type>::GenericInputStream() {

        }

        /**
         * @details Creates a new GenericInputStream and connects it to the inputStreamEventHandler
         * @tparam Type The type of data the stream is handling
         * @param inputStreamEventHandler The inputStreamEventHandler to handle the data
         */
        template<class Type> GenericInputStream<Type>::GenericInputStream(InputStreamEventHandler<Type> *inputStreamEventHandler) {

            // Connect the inputStreamEventHandler
            connectEventHandler(inputStreamEventHandler);

        }

        /**
         * @details Destroys the GenericInputStream and disconnects all internetProtocolHandlers
         * @tparam Type The type of data the stream is handling
         */
        template<class Type> GenericInputStream<Type>::~GenericInputStream() {

            // Disconnect all handlers
            while (!inputStreamEventHandlers.empty())
                diconnectEventHandler(*(inputStreamEventHandlers.begin()));
        }

        /**
         * @details Adds a inputStreamEventHandler to the list of internetProtocolHandlers
         * @tparam Type The type of data the stream is handling
         * @param inputStreamEventHandler The inputStreamEventHandler to add
         */
        template<class Type> void GenericInputStream<Type>::connectEventHandler(InputStreamEventHandler<Type> *inputStreamEventHandler) {

            // Check if the inputStreamEventHandler is already connected
            if(inputStreamEventHandlers.find(inputStreamEventHandler) != inputStreamEventHandlers.end())
                return;

            // Add the inputStreamEventHandler to the list of handlers
            inputStreamEventHandlers.pushBack(inputStreamEventHandler);

        }

        /**
         * @details Removes a inputStreamEventHandler from the list of internetProtocolHandlers
         * @tparam Type The type of data the stream is handling
         * @param inputStreamEventHandler The inputStreamEventHandler to remove
         */
        template<class Type> void GenericInputStream<Type>::diconnectEventHandler(InputStreamEventHandler<Type> *inputStreamEventHandler) {

            // Check if the inputStreamEventHandler is connected
            if(inputStreamEventHandlers.find(inputStreamEventHandler) == inputStreamEventHandlers.end())
                return;

            // Remove the inputStreamEventHandler from the list of handlers
            inputStreamEventHandlers.erase(inputStreamEventHandler);

            // Fire the end of stream event
            inputStreamEventHandler -> onEndOfStream(this);

        }



    }

}

#endif //MAXOS_INPUTSTREAM_H
