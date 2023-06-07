const express = require("express");
const axios = require("axios")

const port = 3000;
const app = express();
app.use(express.json());

const endHost = "https://api.openai.com"

app.get('*', async (req, res) => {
  const fullUrl = new URL(req.path, endHost).href

  const config = {
    headers: { "Authorization": req.headers.authorization }
  }

  const response = await axios.get(fullUrl, config)

  res.status(200).send(JSON.stringify(response.data))
});


app.post('*', async (req, res) => {
  const fullUrl = new URL(req.path, endHost).href

  const config = {
    headers: { "Authorization": req.headers.authorization }
  }

  const response = await axios.post(fullUrl, req.body, config)

  res.status(200).send(JSON.stringify(response.data))
});

// Start the server
app.listen(port, () => {
  console.log(`Server is listening on port ${port}`);
});
