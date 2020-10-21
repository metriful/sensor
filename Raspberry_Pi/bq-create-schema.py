from configparser import ConfigParser
from pathlib import Path
from google.cloud import bigquery
from google.oauth2 import service_account


# Read the config and credentials files
# ---------------------------
config = ConfigParser()
config.read('{}/.metriful'.format(str(Path.home())))

# Read the Google service account credentials
key_path = '{}/.metriful-service-account.json'.format(str(Path.home()))

credentials = service_account.Credentials.from_service_account_file(
    key_path, scopes=["https://www.googleapis.com/auth/cloud-platform"],
)

# Set table_id to the ID of the table to create.
bq_dataset_id = config.get('main', 'bq_dataset_id')
bq_table_id = config.get('main', 'bq_table_id')
bq_table_id_combined = '{}.{}.{}'.format(credentials.project_id, bq_dataset_id, bq_table_id)


# Initialize the client
# ---------------------------
client = bigquery.Client(credentials=credentials, project=credentials.project_id,)


# First create the dataset
# ---------------------------
# Set dataset_id to the ID of the dataset to create.
dataset_id = "{}.{}".format(client.project, bq_dataset_id)

# Construct a full Dataset object to send to the API.
dataset = bigquery.Dataset(dataset_id)

# TODO(developer): Specify the geographic location where the dataset should reside.
dataset.location = "US"

# Send the dataset to the API for creation, with an explicit timeout.
# Raises google.api_core.exceptions.Conflict if the Dataset already
# exists within the project.
dataset = client.create_dataset(dataset, timeout=30)  # Make the API request
print("Created dataset {}.{}".format(client.project, dataset.dataset_id))


# Next create the table and schema
# ---------------------------
schema=[
    bigquery.SchemaField("aqi", "FLOAT"),
    bigquery.SchemaField("aqi_string", "STRING"),
    bigquery.SchemaField("bvoc", "FLOAT"),
    bigquery.SchemaField("humidity", "FLOAT"),
    bigquery.SchemaField("illuminance", "FLOAT"),
    bigquery.SchemaField("location", "STRING"),
    bigquery.SchemaField("particulates", "FLOAT"),
    bigquery.SchemaField("peak_amp", "FLOAT"),
    bigquery.SchemaField("pressure", "INTEGER"),
    bigquery.SchemaField("spl", "FLOAT"),
    bigquery.SchemaField("temperature", "FLOAT"),
    bigquery.SchemaField("temperature_f", "FLOAT"),
    bigquery.SchemaField("timestamp", "TIMESTAMP"),
]

table = bigquery.Table(bq_table_id_combined, schema=schema)
table = client.create_table(table)  # Make the API request
print(
    "Created table {}.{}.{}".format(table.project, table.dataset_id, table.table_id)
)
