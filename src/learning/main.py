import torch
from torch import nn
import matplotlib.pyplot as plt
from typing import List

from dataloader import MyDataset

DATASET_PATH = "exp2"
dataset = MyDataset("dataset/" + DATASET_PATH)

dataset_size = len(dataset)
# 学習データ、検証データに 8:2 の割合で分割する。
train_size = int(0.8 * dataset_size)
val_size = dataset_size - train_size

train_dataset, val_dataset = torch.utils.data.random_split(
    dataset, [train_size, val_size]
)

batch_size = 32

train_dataloader = torch.utils.data.DataLoader(
     dataset=train_dataset,
     batch_size=batch_size,
     shuffle=True, 
     # num_workers=2
 )

test_dataloader = torch.utils.data.DataLoader(
    dataset=val_dataset,
    batch_size=batch_size,
    shuffle=True, 
    # num_workers=2
)

def _get_output_size()->int:
    for a in train_dataloader:
        return len(a[0][0])
    return 0

OUTPUT_SIZE = _get_output_size()

class NeuralNetwork(nn.Module):
    def __init__(self):
        super().__init__()
        self.flatten = nn.Flatten()
        self.linear_relu_stack = nn.Sequential(
            nn.Linear(OUTPUT_SIZE, 512),
            nn.ReLU(),
            nn.Linear(512, 256),
            nn.ReLU(),
            nn.Linear(256, 6),
        )

    def forward(self, x):
        x = self.flatten(x)
        logits = self.linear_relu_stack(x)
        return logits

model = NeuralNetwork()


def train_loop(dataloader, model, loss_fn, optimizer):
    size = len(dataloader.dataset)
    # Set the model to training mode - important for batch normalization and dropout layers
    # Unnecessary in this situation but added for best practices
    model.train()
    loss_: List[float]= []
    for batch, (X, y) in enumerate(dataloader):
        # Compute prediction and loss
        pred = model(X)
        loss = loss_fn(pred, y)
        # Backpropagation
        loss.backward()
        optimizer.step()
        optimizer.zero_grad()

        if batch % 100 == 0:
            loss, current = loss.item(), (batch + 1) * len(X)
            loss_.append(loss)
            print(f"loss: {loss:>7f}  [{current:>5d}/{size:>5d}]")

    return loss_

def test_loop(dataloader, model, loss_fn):
    # Set the model to evaluation mode - important for batch normalization and dropout layers
    # Unnecessary in this situation but added for best practices
    model.eval()
    size = len(dataloader.dataset)
    num_batches = len(dataloader)
    test_loss, correct = 0, 0

    # Evaluating the model with torch.no_grad() ensures that no gradients are computed during test mode
    # also serves to reduce unnecessary gradient computations and memory usage for tensors with requires_grad=True
    with torch.no_grad():
        for X, y in dataloader:
            pred = model(X)
            test_loss += loss_fn(pred, y).item()
            # correct += (pred.argmax(1) == y).type(torch.float).sum().item()

    test_loss /= num_batches
    # correct /= size
    print(f"Test Error: \n Accuracy: {(100*correct):>0.1f}%, Avg loss: {test_loss:>8f} \n")

    return test_loss
    
learning_rate = 2e-3

# https://qiita.com/lucasta1/items/3e0e4306940fc35e0af1
# loss_fn = nn.KLDivLoss(reduction='batchmean')
loss_fn = nn.MSELoss()
optimizer = torch.optim.SGD(model.parameters(), lr=learning_rate)

epochs = 20

train_loss :List[float]= []
test_loss = []

for t in range(epochs):
    print(f"Epoch {t+1}\n-------------------------------")
    tr_loss:List[float] = train_loop(train_dataloader, model, loss_fn, optimizer)
    test_los = test_loop(test_dataloader, model, loss_fn)
    train_loss.extend(tr_loss)
    test_loss.append(test_los)
print("Done!")

x = [i*epochs / len(train_loss) for i in range(len(train_loss))]

filename = "model_weight" + DATASET_PATH + ".pth"
torch.save(model.state_dict(), filename)

PLOT = True
if PLOT:
    plt.plot(x, train_loss, label="train_loss")
    plt.plot([i for i in range(epochs)], test_loss, label="test_loss")
    plt.xlabel("epoch")
    plt.legend()
    plt.ylabel("MSE loss")
    plt.grid()
    plt.show()

print("Final loss, test loss: ", test_loss[-1], "train loss: ", train_loss[-1])

