# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "marimo",
#     "matplotlib==3.10.0",
#     "numpy==2.2.0",
#     "snoop==0.6.0",
#     "torch==2.5.1",
#     "torchvision==0.20.1",
# ]
# ///

import marimo

__generated_with = "0.10.6"
app = marimo.App(width="medium")


@app.cell
def _():
    import marimo as mo
    import torch
    import torch.nn as nn
    from torchvision import datasets, transforms
    import matplotlib.pyplot as plt
    import numpy as np
    import snoop
    return datasets, mo, nn, np, plt, snoop, torch, transforms


@app.cell
def _(datasets, torch, transforms):
    transform = transforms.Compose([transforms.ToTensor(), transforms.Normalize((0.5,), (0.5,))])
    trainset = datasets.MNIST('mnist_train', train=True, download=True, transform=transform)
    trainloader = torch.utils.data.DataLoader(trainset, batch_size=64, shuffle=True)
    testset = datasets.MNIST('mnist_train', train=False, download=True, transform=transform)
    testloader = torch.utils.data.DataLoader(testset)
    return testloader, testset, trainloader, trainset, transform


@app.cell
def _(plt, trainset):
    plt.imshow(trainset.data[5].numpy(), cmap='gray')
    plt.show()
    return


@app.cell
def _(nn):
    class Net(nn.Module):
        def __init__(self, input_size, hidden_size, num_classes):
            super(Net, self).__init__()
            self.fc1 = nn.Linear(input_size, hidden_size) 
            self.relu = nn.ReLU()
            self.fc2 = nn.Linear(hidden_size, num_classes)  

        def forward(self, x):
            out = self.fc1(x)
            out = self.relu(out)
            out = self.fc2(out)
            return out
    return (Net,)


@app.cell
def _(Net):
    net = Net(784, 64, 10)
    net
    return (net,)


@app.cell
def _(net, nn, torch):
    criterion = nn.CrossEntropyLoss()  
    optimizer = torch.optim.Adam(net.parameters(), lr=0.001)
    return criterion, optimizer


@app.cell
def _(torch):
    def rightness(predictions, labels):
        pred = torch.max(predictions.data, 1)[1] 
        rights = pred.eq(labels.data.view_as(pred)).sum() 
        return rights, len(labels)
    return (rightness,)


@app.cell
def _(criterion, net, optimizer, trainloader):
    def train(epochs):
        for epoch in range(epochs):  # loop over the dataset multiple times
            for i, (inputs, labels) in enumerate(trainloader):
                inputs = inputs.view(inputs.size(0), -1)
                optimizer.zero_grad()
                outputs = net(inputs)
                loss = criterion(outputs, labels)
                loss.backward()
                optimizer.step()
                if (i+1) % 200 == 0:
                    print ('Epoch [{:d}/{:d}], Step [{:3d}], Loss: {:.4f}'.format( 
                         epoch+1, epochs, i+1, loss.data))
    return (train,)


@app.cell
def _(train):
    train(10)
    return


@app.cell
def _(net, testloader, torch):
    def test():
        correct = 0
        total = 0
        for i, (inputs, labels) in enumerate(testloader):
            inputs = inputs.view(inputs.size(0), -1)
            outputs = net(inputs)
            _, predicted = torch.max(outputs.data, 1)
            total += labels.size(0)
            correct += (predicted == labels).sum()
        return torch.true_divide(correct, total)
    return (test,)


@app.cell
def _(test):
    test()
    return


@app.cell
def _(net):
    net.state_dict()
    return


@app.cell
def _(net, np):
    def save():
        state = net.state_dict()
        names = ['fc1.weight', 'fc1.bias', 'fc2.weight', 'fc2.bias']
        for name in names:
            np.savetxt(f"{name}.txt", state[name])
    return (save,)


@app.cell
def _(save):
    save()
    return


@app.cell
def _(plt, testset):
    for i in range(10, 26):
        plt.imshow(testset.data[i].numpy(), cmap='gray')
        plt.title(f"{testset.targets[i]}")
        plt.show()
    return (i,)


@app.cell
def _(np, testset):
    def save_data():
        delta = 10
        num = 16
        tests = []
        for i in range(delta, delta+num):
            pic = testset.data[i].numpy()
            pic = pic.flatten()
            tests.append(pic)
        arr = np.array(tests).transpose()
        np.savetxt(f"data.txt", arr)
        labels = np.array([testset.targets[i] for i in range(delta, delta+num)])
        np.savetxt(f"labels.txt", labels, fmt="%.0f")
    return (save_data,)


@app.cell
def _(save_data):
    save_data()
    return


@app.cell
def _():
    16**4
    return


@app.cell
def _(size):
    size(64)
    return


@app.cell
def _():
    def size(t):
        return 16*784+784*t+16*t+t*16+16*16
    return (size,)


if __name__ == "__main__":
    app.run()
