import azure.cognitiveservices.speech as speechsdk
from enum import Enum

class VoiceName(Enum):
    # Add voices here. Link to the Voice library: https://speech.microsoft.com/portal/voicegallery
    Jenny       = "en-US-JennyMultilingualNeural"
    Prabhat     = "en-IN-PrabhatNeural"

def generate_xml(text, voiceName, volume=None, rate=None, pitch=None):
    xml = f'''<speak version="1.0" xmlns="http://www.w3.org/2001/10/synthesis"
    xmlns:mstts="https://www.w3.org/2001/mstts" xml:lang="en-Us">
    <voice name="{voiceName}">
        {text}
    </voice>
</speak>
    '''
    return xml

def configureMicrosoftazure():
    # This requires environment variables named "SPEECH_KEY" and "SPEECH_REGION"
    speech_key, service_region = "360d2c59a64c44cebce83a1c3feeccc0", "germanywestcentral" 
    speech_config = speechsdk.SpeechConfig(subscription=speech_key, region=service_region)

    audio_config = speechsdk.audio.AudioOutputConfig(use_default_speaker=True)

    # The language of the voice that speaks.
    speech_config.speech_synthesis_voice_name='en-US-JennyMultilingualNeural'

    speech_synthesizer = speechsdk.SpeechSynthesizer(speech_config=speech_config, audio_config=audio_config)
    
    return speech_synthesizer


def generateSpeech(text, debug_print=False):

    speech_synthesizer = configureMicrosoftazure()

    speech_synthesis_result = speech_synthesizer.speak_ssml_async(text).get()
    stream = speechsdk.AudioDataStream(speech_synthesis_result)

    if debug_print:
        if speech_synthesis_result.reason == speechsdk.ResultReason.SynthesizingAudioCompleted:
            print("Speech synthesized for text [{}]".format(text))
        elif speech_synthesis_result.reason == speechsdk.ResultReason.Canceled:
            cancellation_details = speech_synthesis_result.cancellation_details
            print("Speech synthesis canceled: {}".format(cancellation_details.reason))
            if cancellation_details.reason == speechsdk.CancellationReason.Error:
                if cancellation_details.error_details:
                    print("Error details: {}".format(cancellation_details.error_details))
                    print("Did you set the speech resource key and region values?")

    return stream

def main():
    # Get text from the console and synthesize to the default speaker.
    print("Enter some text that you want to speak >")
    text = input()

    # Generate XML for the provided word using the specified language and voice
    xml = generate_xml(text, voiceName=VoiceName.Prabhat.value)
    # Generate speech for the given word using the generated XML
    print(xml)
    generateSpeech(xml)

# Run the main function if the script is executed directly
if __name__ == "__main__":
    main()
