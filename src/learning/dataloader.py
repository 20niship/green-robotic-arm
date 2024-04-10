import torch
import time
import pickle
import numpy as np
import glob

def timestamp():
    t = time.time()
    return time.strftime('%Y%m%d-%H-%M-%S', time.localtime(t))

without_vel = True

class MyDataset(torch.utils.data.Dataset):
    noise_rate = 0.02
    after_n = 200 # 何フレーム後のデータを予測するか
    average = 10 # 何フレームの平均を取るか

    AXIS_POS_INDEX = [6, 8, 10, 12, 14, 16]
    AXIS_VEL_INDEX = [7, 9, 11, 13, 15, 17]

    def __init__(self, folder: str):
        self._features_values = []
        self._labels = []

        self.folder = folder
        files = glob.glob(folder + '/*.csv')
        for f in files:
            self._load_files(f)

        self._add_noise()
        self._add_noise()
        self._add_noise()
        self._add_noise()
        print("All data loaded. ", len(self._features_values), len(self._labels))

        self._features_values = np.array(self._features_values, dtype=np.float32)
        self._labels = np.array(self._labels, dtype=np.float32)

        if without_vel:
            self._features_values = np.delete(self._features_values, self.AXIS_VEL_INDEX, axis=1)
            self.AXIS_POS_INDEX = [6, 7, 8, 9, 10, 11]

        self.feature_mean = np.mean(self._features_values, axis=0)
        self.feature_std = np.std(self._features_values, axis=0)
        self.label_mean = np.mean(self._labels, axis=0)
        self.label_std = np.std(self._labels, axis=0)

        self._features_values = (self._features_values - self.feature_mean) / self.feature_std 
        self._labels = (self._labels - self.label_mean) / self.label_std
        self._labels = self._labels[:, self.AXIS_POS_INDEX]

        self.dump_pickle()

    def dump_pickle(self): 
        input_files_raw = []
        index : int = 0
        for f in glob.glob(self.folder + '/*.csv'):
            data = np.loadtxt(f, delimiter=',', skiprows=1, usecols=set(range(1, 19)))
            if without_vel:
                data = np.delete(data, self.AXIS_VEL_INDEX, axis=1)
            data = (data - self.feature_mean) / self.feature_std
            data = np.insert(data, 0, index, axis=1)
            index += 1
            input_files_raw.extend(data)

        input_db = {
            "name": "学習データセット",
            "timestamp": timestamp(),
            "data_raw": input_files_raw,
            "input": self._features_values.tolist(),
            "output": self._labels.tolist(),
            "mean-input": np.mean(self._features_values, axis=0).tolist(),
            "mean-output": np.mean(self._labels, axis=0).tolist(),
            "std-input": np.std(self._features_values, axis=0).tolist(),
            "std-output": np.std(self._labels, axis=0).tolist(),
            "experiment": self.folder,
        }
        pickle.dump(input_db, open("input_db" + timestamp() + ".pkl", "wb"))

    def _load_files(self, file: str):
        usecols = ()
        if "exp1" in file:
            usecols = set(range(1, 15))
        else:
            usecols = set(range(1, 19))
        data = np.loadtxt(file, delimiter=',', skiprows=1, usecols=usecols)
        # assert data.shape[1] == 18

        i = 0
        while i + self.after_n + self.average < len(data):
            input_ = np.average(data[i:i+self.average, :], axis=0)
            output_ = np.average(data[i+self.average:i+self.average+self.after_n, :], axis=0)

            # assert len(input_) == 18
            # assert len(output_) == 18

            self._features_values.append(input_)
            self._labels.append(output_)
            i += self.after_n

    def _add_noise(self):
        noise_in  = []
        noise_out = []
        for _ in range(2):
            noise_added = np.random.normal(0, self.noise_rate, np.array(self._features_values).shape) + self._features_values
            ans = np.random.normal(0, self.noise_rate, np.array(self._labels).shape) + self._labels
            noise_in.extend(noise_added)
            noise_out.extend(ans)
        self._features_values.extend(noise_in)
        self._labels.extend(noise_out)


    def __len__(self):
        return len(self._features_values)

    def __getitem__(self, idx):
        features_x = torch.FloatTensor(self._features_values[idx])
        labels = torch.FloatTensor(self._labels[idx])
        return features_x, labels


def _sample():
    dataset = MyDataset("dataset/exp2")
    dl = torch.utils.data.DataLoader(
             dataset=dataset,
             batch_size=100, 
             shuffle=True, 
             # num_workers=2
             )

    X_list, y_list = [], []
    for _ , (X, y) in enumerate(dl):
        X_list.append(X)
        y_list.append(y)
    print('len:', len(X_list), len(y_list))


if __name__ == '__main__':
    _sample()
