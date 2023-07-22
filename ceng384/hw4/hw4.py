import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
import scipy.io

# fast fourier transformation function
def fft(x):
    x = np.asarray(x, dtype=float)
    N = x.shape[0]
    n = np.arange(N)
    k = n.reshape((N,1))
    M = np.exp(-2j * np.pi * k * n / N)
    return np.dot(M, x)

# inverse fast fourier transformation function
def ifft(X):
    X = np.asarray(X, dtype=complex)
    N = X.shape[0]
    n = np.arange(N)
    k = n.reshape((N,1))
    M = np.exp(2j * np.pi * k * n / N)
    return np.dot(M, X) / N

# get data
path = r"/content/encoded.wav"
samplerate, data = wavfile.read(path)
length = data.shape[0] / samplerate

# obtain fourier domain representation of the signal
fourier_domain_signal = fft(data)

# apply decoding recipe step 2
X_prime_1 = fourier_domain_signal[:len(fourier_domain_signal)//2]
X_prime_2 = fourier_domain_signal[len(fourier_domain_signal)//2:]

X_1 = []
for i in range(1, len(X_prime_1)+1):
    X_1.append(X_prime_1[-i])
X_2 = []
for i in range(1, len(X_prime_2)+1):
    X_2.append(X_prime_2[-i])

X_prime = np.concatenate((X_1, X_2), axis=None)

# return to time domain
time_domain_signal = ifft(X_prime)

# convert signal to wav file
wavfile.write("decoded_message.wav", samplerate, time_domain_signal.astype(np.int16))

# plot function
def plot(data_to_plot):
    time = np.linspace(0., length, data_to_plot.shape[0])

    plt.plot(time, data_to_plot[:], label="signal")

    plt.legend()

    plt.xlabel("Time [s]")

    plt.ylabel("Amplitude")

    plt.show()
