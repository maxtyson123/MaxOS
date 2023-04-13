//
// Created by 98max on 13/04/2023.
//

#ifndef MAXOS_COMMON_OUTPUTSTREAM_H
#define MAXOS_COMMON_OUTPUTSTREAM_H

#include <common/types.h>
#include <common/inputStream.h>

namespace maxOS{

    namespace common{

        template<class Type> class GenericOutputStream : public InputStreamProcessor<Type,Type>
        {
            public:
                GenericOutputStream();
                ~GenericOutputStream();

                void onRead(Type readElement);
                void onEndOfStream(GenericInputStream<Type>* stream);

                virtual void write(Type writeElement);
                virtual void close();

                virtual GenericOutputStream<Type>& operator << (Type writeElement);
        };

        class OutputStream : public GenericOutputStream<string>
        {

            public:
                OutputStream();
                ~OutputStream();

                virtual void lineFeed();
                virtual void carriageReturn();
                virtual void clear();

                virtual void write(string stringToWrite);
                virtual void writeChar(char charToWrite);
                virtual void writeInt(int intToWrite);
                virtual void writeHex(uint32_t hexToWrite);

                OutputStream& operator << (int intToWrite);
                OutputStream& operator << (uint32_t hexToWrite);
                OutputStream& operator << (string stringToWrite);
                OutputStream& operator << (char charToWrite);
        };



        ///__________________________________________Templates__________________________________________________///

        /**
         * @details Constructor of the GenericOutputStream class.
         * @tparam Type The type of the elements that will be written to the stream.
         */
        template<class Type> GenericOutputStream<Type>::GenericOutputStream() {

        }

        /**
         * @details Destructor of the GenericOutputStream class.
         * @tparam Type The type of the elements that will be written to the stream.
         */
        template<class Type> GenericOutputStream<Type>::~GenericOutputStream() {

        }

        /**
         * @details This function is called when an element is read from the stream.
         * @tparam Type The type of the elements that will be written to the stream.
         * @param readElement The element that was read from the stream.
         */
        template<class Type> void GenericOutputStream<Type>::onRead(Type readElement) {

            // When something is read from the input stream, it is written to the output stream.
            write(readElement);

            // Pass the element to any handlers.
            for(typename  Vector<InputStreamEventHandler<Type>*>::iterator inputStreamEventHandler = this -> inputStreamEventHandlers.begin(); inputStreamEventHandler != this -> inputStreamEventHandlers.end(); ++inputStreamEventHandler)
                (*inputStreamEventHandler) -> onRead(readElement);


        }

        /**
         * @details This function is called when the end of the stream is reached.
         * @tparam Type The type of the elements that will be written to the stream.
         * @param stream The stream that reached the end.
         */
        template<class Type> void GenericOutputStream<Type>::onEndOfStream(GenericInputStream<Type> *stream) {

            // Close the stream.
            close();

            // Pass the event to any handlers.
            for(typename  Vector<InputStreamEventHandler<Type>*>::iterator inputStreamEventHandler = this -> inputStreamEventHandlers.begin(); inputStreamEventHandler != this -> inputStreamEventHandlers.end(); ++inputStreamEventHandler)
                (*inputStreamEventHandler) -> onEndOfStream(stream);

            // Handle the event on this class
            InputStreamProcessor<Type, Type>::onEndOfStream(stream);

        }

        /**
         * @details This function writes an element to the stream.
         * @tparam Type The type of the elements that will be written to the stream.
         * @param writeElement The element that will be written to the stream.
         */
        template<class Type> void GenericOutputStream<Type>::write(Type writeElement) {

        }

        /**
         * @details This function closes the stream.
         * @tparam Type The type of the elements that will be written to the stream.
         */
        template<class Type> void GenericOutputStream<Type>::close() {

        }

        /**
         * @details This function writes an element to the stream.
         * @tparam Type The type of the elements that will be written to the stream.
         * @param writeElement The element that will be written to the stream.
         * @return The stream.
         */
        template<class Type> GenericOutputStream<Type> &GenericOutputStream<Type>::operator << (Type writeElement) {

            // Write the element to the stream.
            write(writeElement);

            // Return the stream.
            return *this;

        }

    }

}


#endif //MAXOS_COMMON_OUTPUTSTREAM_H
