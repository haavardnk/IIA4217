using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Data.SqlClient;

namespace Lab1.Models
{
    public class Measurement
    {
        public int MeasurementDataId { get; set; }
        public string MeasurementName { get; set; }
        public string MeasurementTimeStamp { get; set; }
        public double MeasurementValue { get; set; }
        public string MeasurementUnit { get; set; }

        public List<Measurement> GetMeasurements(string connectionString)
        {
            List<Measurement> measurementList = new List<Measurement>();
            SqlConnection con = new SqlConnection(connectionString);
            string sqlQuery = "SELECT * from GetMeasurementData";
            con.Open();
            SqlCommand cmd = new SqlCommand(sqlQuery, con);
            SqlDataReader dr = cmd.ExecuteReader();

            if (dr != null)
            {
                while (dr.Read())
                {
                    Measurement measurement = new Measurement();

                    measurement.MeasurementDataId = Convert.ToInt32(dr["MeasurementDataId"]);
                    measurement.MeasurementName = Convert.ToString(dr["MeasurementName"]);
                    measurement.MeasurementTimeStamp = Convert.ToString(dr["MeasurementTimeStamp"]);
                    measurement.MeasurementValue = Convert.ToDouble(dr["MeasurementValue"]);
                    measurement.MeasurementUnit = Convert.ToString(dr["MeasurementUnit"]);

                    measurementList.Add(measurement);
                }
            }
            return measurementList;
        }

    }
}
